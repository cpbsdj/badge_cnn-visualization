# workspace/data/build_clean_dataset.py
import pathlib, random
from PIL import Image, ImageOps, ImageEnhance, ImageFilter

raw_dir     = pathlib.Path("/workspace/data/raw")   # 512*512 RGBA 彩图
out_dir     = pathlib.Path("clean")                   # 输出 64*64 灰度
target_size = 64
num_per_cls = 200
train_n, val_n = 140, 30

out_dir.mkdir(exist_ok=True)
for phase in ["train", "val", "test"]:
    (out_dir/phase).mkdir(exist_ok=True)

classes = ['pku','thu','hit','fdu','sjtu','ustc','nju','xjtu','zju']

def full_augment(im: Image.Image):
    """几何+光度+模糊 全套"""
    im = im.convert("RGBA")

    # 1. 随机缩放 + 旋转
    scale = random.uniform(0.65, 1.0)
    w, h = im.size
    new_w, new_h = int(w * scale), int(h * scale)
    im = im.resize((new_w, new_h), Image.LANCZOS)
    im = im.rotate(random.uniform(-35, 35), expand=True, fillcolor=(0, 0, 0, 0))

    # 2. 颜色抖动（亮度 / 对比度 / 饱和度）
    im = ImageEnhance.Brightness(im).enhance(random.uniform(0.7, 1.3))
    im = ImageEnhance.Contrast(im).enhance(random.uniform(0.7, 1.3))
    # 饱和度只对 RGB 层有效，这里转成 RGB 再抖一次
    rgb = Image.new("RGB", im.size, (255, 255, 255))
    rgb.paste(im, mask=im.split()[3])        # 用 alpha 做 mask 贴到白底
    rgb = ImageEnhance.Color(rgb).enhance(random.uniform(0.6, 1.4))

    # 3. 随机高斯模糊
    if random.random() < 0.35:
        rgb = rgb.filter(ImageFilter.GaussianBlur(radius=random.uniform(0, 1.2)))

    # 4. 随机 JPEG 压缩噪声（保存再读取）
    if random.random() < 0.3:
        tmp = pathlib.Path(f"/tmp/jpg_{random.randint(0,99999)}.jpg")
        rgb.save(tmp, quality=random.randint(65, 95))
        rgb = Image.open(tmp)
        tmp.unlink(missing_ok=True)

    # 5. 统一 64×64 灰度
    gray = ImageOps.pad(rgb, (target_size, target_size), color=255).convert("L")
    return gray


for cls in classes:
    src_png = raw_dir / f"{cls}.png"
    if not src_png.exists():
        print("skip", cls); continue
    all_imgs = []
    for i in range(num_per_cls):
        all_imgs.append(full_augment(Image.open(src_png)))
    random.shuffle(all_imgs)
    splits = {"train": all_imgs[:train_n],
              "val"  : all_imgs[train_n:train_n + val_n],
              "test" : all_imgs[train_n + val_n:]}
    for phase, imgs in splits.items():
        (out_dir / phase / cls).mkdir(exist_ok=True)
        for idx, im in enumerate(imgs):
            im.save(out_dir / phase / cls / f"{idx:03d}.png")
    print(cls, "done")
print("full-augment clean dataset ready at", out_dir.resolve())

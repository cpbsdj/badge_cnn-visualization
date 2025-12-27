import pathlib, shutil, random
from PIL import Image

raw_dir   = pathlib.Path("data/raw")
split_dir = pathlib.Path("data/split")
target_size = 64
train_ratio = 0.7
val_ratio   = 0.15
test_ratio  = 0.15
assert abs(sum([train_ratio, val_ratio, test_ratio]) - 1) < 1e-5

split_dir.mkdir(exist_ok=True)
for phase in ["train", "val", "test"]:
    (split_dir/phase).mkdir(exist_ok=True)

classes = sorted([d.name for d in raw_dir.iterdir() if d.is_dir()])
label2idx = {c:i for i,c in enumerate(classes)}
(pathlib.Path("data/label2idx.json")).write_text(str(label2idx).replace("'", '"'))

for cls in classes:
    src_imgs = list((raw_dir/cls).glob("*"))
    random.shuffle(src_imgs)
    n = len(src_imgs)
    n_train = int(n*train_ratio)
    n_val   = int(n*val_ratio)
    splits = {"train": src_imgs[:n_train],
              "val"  : src_imgs[n_train:n_train+n_val],
              "test" : src_imgs[n_train+n_val:]}
    for phase, imgs in splits.items():
        (split_dir/phase/cls).mkdir(exist_ok=True)
        for img_path in imgs:
            try:
                img = Image.open(img_path).convert("L").resize((target_size, target_size), Image.LANCZOS)
                save_path = split_dir/phase/cls/img_path.name
                img.save(save_path)
            except Exception as e:
                print("skip", img_path, e)
print("split done!")

#!/usr/bin/env python3
import argparse, os, sys, time
import torch
import torchvision.transforms as T
from PIL import Image
from model.badge_cnn import BadgeCNN

# 类别顺序必须与训练时 ImageFolder 的 `class_to_idx` 一致
CLASS_NAMES = ['fdu', 'hit', 'nju', 'pku', 'sjtu', 'thu', 'ustc', 'xjtu', 'zju']

def load_model(weight_path, device):
    model = BadgeCNN(num_classes=len(CLASS_NAMES)).to(device)
    model.load_state_dict(torch.load(weight_path, map_location=device))
    model.eval()
    return model

def infer_one(path, model, transform, device):
    img = Image.open(path).convert('L')          # 强制灰度
    x = transform(img).unsqueeze(0).to(device)   # 1×1×64×64
    with torch.no_grad():
        logits = model(x)
        prob   = torch.softmax(logits, dim=1)
        score, pred = prob.max(1)
    cls = CLASS_NAMES[pred.item()]
    return cls, score.item()

def main():
    parser = argparse.ArgumentParser(description='校徽 9 分类推理')
    parser.add_argument('--weight', default='ckpts/badge9_best.pth', help='模型路径')
    parser.add_argument('--img',    required=True, help='单张图片或文件夹路径')
    parser.add_argument('--topk',   type=int, default=1, help='显示前 k 个预测')
    args = parser.parse_args()

    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    print(f'device: {device}')

    transform = T.Compose([
        T.Resize(64),
        T.ToTensor(),
        T.Normalize((0.5,), (0.5,))
    ])

    model = load_model(args.weight, device)

    # 单张infer或批量infer
    img_path = args.img
    if os.path.isfile(img_path):
        cls, score = infer_one(img_path, model, transform, device)
        print(f'{img_path}  -->  {cls}  ({score*100:.1f}%)')
    else:
        files = [os.path.join(img_path, f) for f in os.listdir(img_path)
                 if f.lower().endswith(('.png', '.jpg', '.jpeg'))]
        print(f'find {len(files)} images\n')
        for fp in files:
            cls, score = infer_one(fp, model, transform, device)
            print(f'{os.path.basename(fp):<25}  -->  {cls}  ({score*100:.1f}%)')

if __name__ == '__main__':
    main()

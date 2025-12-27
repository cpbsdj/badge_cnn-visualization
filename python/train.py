# train.py
import torch, os, argparse, tqdm, random,sys, numpy as np
from torch import nn
from torchvision import datasets, transforms
from model.badge_cnn import BadgeCNN


sys.path.append(os.path.dirname(os.path.abspath(__file__)))





def run(args):
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print("device:", device)

    # 1. 数据
    tf = transforms.Compose([
       transforms.Grayscale(num_output_channels=1),  # RGB→灰度
       transforms.ToTensor(),
       transforms.Normalize((0.5,), (0.5,))
    ])

    train_set = datasets.ImageFolder(os.path.join(args.data_dir, "train"), tf)
    val_set   = datasets.ImageFolder(os.path.join(args.data_dir, "val"),   tf)
    train_loader = torch.utils.data.DataLoader(train_set, batch_size=args.batch, shuffle=True,  num_workers=2)
    val_loader   = torch.utils.data.DataLoader(val_set,   batch_size=args.batch, shuffle=False, num_workers=2)

    # 2. 模型
    model = BadgeCNN(num_classes=9).to(device)
    criterion = nn.CrossEntropyLoss()
    optimizer = torch.optim.Adam(model.parameters(), lr=args.lr)

    # 3. 训练循环
    best_acc = 0.
    for epoch in range(args.epoch):
        model.train()
        running_loss, running_correct, n = 0., 0, 0
        for x, y in tqdm.tqdm(train_loader, desc=f"epoch{epoch}"):
            x, y = x.to(device), y.to(device)
            optimizer.zero_grad()
            out = model(x)
            loss = criterion(out, y)
            loss.backward()
            optimizer.step()

            running_loss += loss.item()*x.size(0)
            running_correct += (out.argmax(1)==y).sum().item()
            n += x.size(0)
        train_loss = running_loss/n
        train_acc  = running_correct/n

        # 验证
        model.eval()
        with torch.no_grad():
            running_correct, n = 0, 0
            for x, y in val_loader:
                x, y = x.to(device), y.to(device)
                out = model(x)
                running_correct += (out.argmax(1)==y).sum().item()
                n += x.size(0)
        val_acc = running_correct/n

        print(f"Epoch{epoch:02d} | loss {train_loss:.4f} | train_acc {train_acc:.3f} | val_acc {val_acc:.3f}")
        if val_acc > best_acc:
            best_acc = val_acc
            os.makedirs(os.path.dirname(args.save), exist_ok=True)
            torch.save(model.state_dict(), args.save)
            print("*** best model saved ***")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--data_dir", default="data/clean")
    parser.add_argument("--batch", type=int, default=64)
    parser.add_argument("--epoch", type=int, default=80)
    parser.add_argument("--lr", type=float, default=1e-3)
    parser.add_argument("--save", default="ckpts/badge9_best.pth")
    args = parser.parse_args()
    run(args)

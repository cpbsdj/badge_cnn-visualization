import torch
import torch.nn as nn
import torch.nn.functional as F

class BadgeCNN(nn.Module):
    def __init__(self, num_classes=9):
        super().__init__()
        # 64 -> 32
        self.conv1 = nn.Sequential(
            nn.Conv2d(1, 16, 3, padding=1),   # 1×64×64 -> 16×64×64
            nn.BatchNorm2d(16),
            nn.ReLU(inplace=True),
            nn.MaxPool2d(2)                   # 16×32×32
        )
        # 32 -> 16
        self.conv2 = nn.Sequential(
            nn.Conv2d(16, 32, 3, padding=1),  # 16×32×32 -> 32×32×32
            nn.BatchNorm2d(32),
            nn.ReLU(inplace=True),
            nn.MaxPool2d(2)                   # 32×16×16
        )
        # 16 -> 8
        self.conv3 = nn.Sequential(
            nn.Conv2d(32, 64, 3, padding=1),  # 32×16×16 -> 64×16×16
            nn.BatchNorm2d(64),
            nn.ReLU(inplace=True),
            nn.MaxPool2d(2)                   # 64×8×8
        )
        # 8 -> 4
        self.conv4 = nn.Sequential(
            nn.Conv2d(64, 64, 3, padding=1),  # 64×8×8 -> 64×8×8
            nn.BatchNorm2d(64),
            nn.ReLU(inplace=True),
            nn.MaxPool2d(2)                   # 64×4×4
        )
        # 全局平均池化：64×4×4 -> 64
        self.gap = nn.AdaptiveAvgPool2d(1)
        # 分类器
        self.classifier = nn.Linear(64, num_classes)

    def forward(self, x):
        x = self.conv1(x)
        x = self.conv2(x)
        x = self.conv3(x)
        x = self.conv4(x)
        x = self.gap(x).flatten(1)   # B×64
        return self.classifier(x)


# ---------------- 测试 ----------------
if __name__ == "__main__":
    net = BadgeCNN()
    dummy = torch.randn(8, 1, 64, 64)   # batch=8
    out = net(dummy)
    print(out.shape)   
    print("参数量：", sum(p.numel() for p in net.parameters()))

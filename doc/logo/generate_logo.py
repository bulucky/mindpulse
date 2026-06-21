import math
import numpy as np
from PIL import Image, ImageDraw

def draw_smooth_line_segment(draw, p1, p2, radius, fill):
    """
    使用高密度圆形笔刷扫掠绘制直线段，确保接缝和边缘绝对平滑，消除锯齿
    """
    x1, y1 = p1
    x2, y2 = p2
    dx = x2 - x1
    dy = y2 - y1
    dist = math.sqrt(dx*dx + dy*dy)
    if dist == 0:
        draw.ellipse([x1 - radius, y1 - radius, x1 + radius, y1 + radius], fill=fill)
        return
    
    # 每0.5个像素绘制一个圆形以保证高密度覆盖
    steps = int(dist * 2.0)
    for i in range(steps + 1):
        t = i / steps
        x = x1 + dx * t
        y = y1 + dy * t
        draw.ellipse([x - radius, y - radius, x + radius, y + radius], fill=fill)

def generate_logo():
    # 1. 渲染尺寸参数 (4x 超采样)
    base_size = 512
    scale = 4
    size = base_size * scale # 2048
    cx, cy = size // 2, size // 2 # 1024, 1024

    # 圆环半径参数
    r_outer = 175 * scale # 700
    r_inner = 152 * scale # 608
    r_mid = (r_outer + r_inner) / 2 # 654
    thickness = r_outer - r_inner # 92
    r_cap = thickness / 2 # 46

    # 创建画布 (RGBA)
    img = Image.new("RGBA", (size, size), (0, 0, 0, 255))
    
    # 2. 构造线性渐变背景 (从左上到右下)
    # 颜色起点: 碧绿 #69ad9b (105, 173, 155)
    # 颜色终点: 薰衣草紫 #61649f (97, 100, 159)
    color_start = np.array([105.0, 173.0, 155.0])
    color_end = np.array([97.0, 100.0, 159.0])

    # 用 numpy 构建像素坐标矩阵并计算距离和角度
    y_idx, x_idx = np.ogrid[0:size, 0:size]
    dx = x_idx - cx
    dy = y_idx - cy
    d = np.sqrt(dx*dx + dy*dy)
    
    # 计算极坐标角度 (单位: 度, 范围: [0, 360))
    theta = np.arctan2(dy, dx) * 180.0 / np.pi
    theta = np.where(theta < 0, theta + 360.0, theta)

    # 黄金比例缺口定义:
    # 缺口中心分别位于: 直下 (90°), 左上 (210°), 右上 (330°)
    # 缺口宽度: 120° * 0.3819660113 = 45.836°
    # 实心段宽度: 120° * 0.6180339887 = 74.164°
    
    # 定义 3 个实心段的弧度范围
    solid_ranges = [
        (112.918, 187.082), # 150° 左右
        (232.918, 307.082), # 270° 左右
        (352.918, 67.082)   # 30° 左右 (跨越 0°/360°)
    ]

    # 判断是否在实心段内的掩膜
    in_solid = np.zeros((size, size), dtype=bool)
    for start, end in solid_ranges:
        if start > end: # 跨越零度
            in_solid |= (theta >= start) | (theta <= end)
        else:
            in_solid |= (theta >= start) & (theta <= end)

    # 圆环掩膜
    in_ring = (d >= r_inner) & (d <= r_outer) & in_solid

    # 计算线性渐变插值因子 t_mask [0, 1] (方向: 左上 (0,0) -> 右下 (size, size))
    # 投影公式: t = (x + y) / (2 * size)
    x_grid, y_grid = np.meshgrid(np.arange(size), np.arange(size))
    t_mask = (x_grid + y_grid) / (2.0 * size)

    # 生成 RGB 渐变矩阵
    r_channel = color_start[0] * (1.0 - t_mask) + color_end[0] * t_mask
    g_channel = color_start[1] * (1.0 - t_mask) + color_end[1] * t_mask
    b_channel = color_start[2] * (1.0 - t_mask) + color_end[2] * t_mask

    # 应用掩膜将圆环写入画布
    img_data = np.array(img)
    img_data[in_ring, 0] = r_channel[in_ring].astype(np.uint8)
    img_data[in_ring, 1] = g_channel[in_ring].astype(np.uint8)
    img_data[in_ring, 2] = b_channel[in_ring].astype(np.uint8)
    img_data[in_ring, 3] = 255
    img = Image.fromarray(img_data)

    # 3. 绘制六个圆角端点 (模拟 stroke-linecap="round")
    draw = ImageDraw.Draw(img)
    endpoints = [112.918, 187.082, 232.918, 307.082, 352.918, 67.082]
    for angle in endpoints:
        rad = angle * math.pi / 180.0
        ex = cx + r_mid * math.cos(rad)
        ey = cy + r_mid * math.sin(rad)
        
        # 获得该坐标下的线性渐变插值
        t = (ex + ey) / (2.0 * size)
        color = tuple((color_start * (1.0 - t) + color_end * t).astype(int))
        
        # 绘制圆点
        draw.ellipse([ex - r_cap, ey - r_cap, ex + r_cap, ey + r_cap], fill=color + (255,))

    # 4. 绘制中心静止的纯白心跳脉搏线
    # 使用超平滑扫掠算法绘制折线，线宽约 80px，带有完美圆润的转角
    ekg_points = [
        (512, 1024),
        (732, 1024),
        (804, 1132),
        (916, 512),
        (1024, 1536),
        (1132, 1096),
        (1208, 1024),
        (1536, 1024)
    ]
    r_ekg = 40 # 心跳线半径
    for i in range(len(ekg_points) - 1):
        draw_smooth_line_segment(draw, ekg_points[i], ekg_points[i+1], r_ekg, (255, 255, 255, 255))

    # 5. 双立方插值缩放到 512x512，实现完美的抗锯齿
    final_img = img.resize((base_size, base_size), resample=Image.Resampling.LANCZOS)
    
    # 转换为 RGB 并保存为 PNG
    final_img = final_img.convert("RGB")
    final_img.save("doc/logo/flat_neon.png", "PNG")
    print("Logo successfully generated at doc/logo/flat_neon.png")

if __name__ == "__main__":
    generate_logo()

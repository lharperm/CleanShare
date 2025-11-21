#!/usr/bin/env python

import argparse
import json
import os
import sys

try:
    from ultralytics import YOLO
except ImportError:
    sys.stderr.write("ERROR: 'ultralytics' not installed. Run: pip install ultralytics\n")
    sys.exit(1)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--model", required=True, help="Path to .pt model")
    parser.add_argument("--image", required=True, help="Path to input image")
    parser.add_argument("--conf", type=float, default=0.25, help="Confidence threshold")
    args = parser.parse_args()

    if not os.path.exists(args.model):
        sys.stderr.write(f"ERROR: model not found: {args.model}\n")
        sys.exit(1)

    if not os.path.exists(args.image):
        sys.stderr.write(f"ERROR: image not found: {args.image}\n")
        sys.exit(1)

    # Load model on CPU
    model = YOLO(args.model)

    # Run detection on CPU
    results = model(args.image, conf=args.conf, device="cpu", verbose=False)[0]

    # Original image size
    h, w = results.orig_shape

    detections = []
    boxes = results.boxes

    if boxes is not None and len(boxes) > 0:
        xyxy = boxes.xyxy.cpu().numpy()
        cls = boxes.cls.cpu().numpy()
        conf = boxes.conf.cpu().numpy()

        for (x1, y1, x2, y2), c, cf in zip(xyxy, cls, conf):
            cf = float(cf)
            if cf < args.conf:
                continue

            x = int(round(x1))
            y = int(round(y1))
            w_box = int(round(x2 - x1))
            h_box = int(round(y2 - y1))

            detections.append(
                {
                    "x": x,
                    "y": y,
                    "w": w_box,
                    "h": h_box,
                    "conf": cf,
                    "cls": int(c),
                }
            )

    out = {
        "width": w,
        "height": h,
        "detections": detections,
    }

    print(json.dumps(out), flush=True)


if __name__ == "__main__":
    main()

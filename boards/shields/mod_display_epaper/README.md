# Replacing the Image Asset

Follow these steps to replace the image used by the UI.

## 1. Prepare the image

Convert your image to the required format:

* Size: **164 × 88 pixels**
* Orientation: **rotated 90 degrees counterclockwise** (Horizontal)
* Format: **PNG**

## 2. Install the required tools

Install the dependencies required by `LVGLImage.py`:

```bash
pip install lz4 pypng
```

Make sure `pngquant` is installed on your system.

## 3. Convert the image for LVGL

Download the LVGL image conversion script: https://github.com/lvgl/lvgl/blob/master/scripts/LVGLImage.py

Convert your image using:

```bash
python LVGLImage.py --cf I1 --ofmt C FILE_PATH
```

Replace `FILE_PATH` with the name and location of your generated image. This generates the LVGL-compatible image data.

## 4. Replace the image asset

Open:

```
widgets/art.c
```

Replace the existing image data with the generated contents from `LVGLImage.py`.

## 5. Update the image reference

Open `peripheral_status.c` and update the image declaration on line 26:

```c
LV_IMG_DECLARE(FILE_NAME);
```

Replace `FILE_NAME` with the name of your generated image.

Then update the image source assignment around line 117:

```c
lv_image_set_src(art, &FILE_NAME);
```

Again, replace `FILE_NAME` with your image name.

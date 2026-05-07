# Replacing image

Convert image to 164x88 horizontal. 90 degrees rotated counterclockwise.

Use [LVGLImage.py](https://github.com/lvgl/lvgl/blob/master/scripts/LVGLImage.py. Requires `pngquant` and `pip install lz4 pypng`

then convert image:
```
python LVGLImage.py --cf I1 --ofmt C ../../Downloads/cat.png
```

replace contents in `widgets/art.c`

in peripheral_status.c change line 26 to `LV_IMG_DECLARE(cat);` where cat is the name of your image

and similarly on line 118 `    lv_image_set_src(art, &cat);`

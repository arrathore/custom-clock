export function loadImage(file) {
    return new Promise((resolve, reject) => {
        const url = URL.createObjectURL(file);
        const image = new Image();

        image.onload = () => {
            URL.revokeObjectURL(url);

            resolve({
                image,
                width: image.naturalWidth,
                height: image.naturalHeight,
            });
        };

        image.onerror = () => {
            URL.revokeObjectURL(url);
            reject(new Error(`Unable to load ${file.name}`));
        };

        image.src = url;
    });
}

// convert 8-bit RGB color to RGB565
export function rgbTo565(r, g, b) {
    const red = (r >> 3) & 0x1f;
    const green = (g >> 2) & 0x3f;
    const blue = (b >> 3) & 0x1f;

    return (red << 11) | (green << 5) | blue;
}

export function hexToRgb(hex) {
  const value = hex.replace("#", "");

  return {
    r: parseInt(value.substring(0, 2), 16),
    g: parseInt(value.substring(2, 4), 16),
    b: parseInt(value.substring(4, 6), 16),
  };
}

// composite image against background color
// convert to RGB565
export function convertToRGB565(imageData, backgroundColor) {
  const { data, width, height } = imageData;
  const pixels = new Uint16Array(data.length / 4);

  const background = hexToRgb(backgroundColor);

  const canvas = document.createElement("canvas");
  canvas.width = width;
  canvas.height = height;

  const context = canvas.getContext("2d");
  const outputImageData = context.createImageData(width, height);
  const outData = outputImageData.data;

  for (let i = 0, pixel = 0; i < data.length; i+= 4, pixel++) {
    const r = data[i];
    const g = data[i + 1];
    const b = data[i + 2];
    const a = data[i + 3] / 255;

    // composite the source pixel over the background
    const compositedR = Math.round(r * a + background.r * (1 - a));
    const compositedG = Math.round(g * a + background.g * (1 - a));
    const compositedB = Math.round(b * a + background.b * (1 - a));
    
    const val = rgbTo565(compositedR, compositedG, compositedB);
    pixels[pixel] = val;
    
    // unpack RGB565 back to 8-bit channels to preview exact converted appearance
    const r5 = (val >> 11) & 0x1f;
    const g6 = (val >> 5) & 0x3f;
    const b5 = val & 0x1f;
    
    outData[i] = Math.round((r5 * 255) / 31);
    outData[i + 1] = Math.round((g6 * 255) / 63);
    outData[i + 2] = Math.round((b5 * 255) / 31);
    outData[i + 3] = data[i + 3];
  }

  context.putImageData(outputImageData, 0, 0);
  
  return {
    rgb565: pixels,
    previewUrl: canvas.toDataURL("image/png"),
  };
}

export function getContrastingColor(hex) {
    const { r, g, b } = hexToRgb(hex);
    const brightness = (r * 289 + g * 587 + b * 114) / 1000;
    return brightness > 128 ? "#000000" : "#ffffff";
}

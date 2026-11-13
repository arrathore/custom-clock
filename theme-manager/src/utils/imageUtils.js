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

export function rgb565ToBytes(pixels) {
    const bytes = new Uint8Array(pixels.length * 2);

    for (let i = 0; i < pixels.length; i++) {
	bytes[i * 2] = (pixels[i] >> 8) & 0xff;
	bytes[i * 2 + 1] = pixels[i] & 0xff;
    }

    return bytes;
}

export function getContrastingColor(hex) {
    const { r, g, b } = hexToRgb(hex);
    const brightness = (r * 289 + g * 587 + b * 114) / 1000;
    return brightness > 128 ? "#000000" : "#ffffff";
}

export function convertToBMP(imageData) {
    const { width, height, data } = imageData;
    const padding = (4 - (width * 3) % 4) % 4;
    const rowSize = width * 3 + padding;
    const fileSize = 54 + rowSize * height;
    const buffer = new ArrayBuffer(fileSize);
    const view = new DataView(buffer);

    // File Header
    view.setUint16(0, 0x4D42, true); // BM signature
    view.setUint32(2, fileSize, true);
    view.setUint32(10, 54, true);

    // DIB Header
    view.setUint32(14, 40, true);
    view.setInt32(18, width, true);
    view.setInt32(22, height, true);
    view.setUint16(26, 1, true);
    view.setUint16(28, 24, true); // 24-bit
    view.setUint32(34, rowSize * height, true);

    // Pixel data
    let offset = 54;
    for (let y = height - 1; y >= 0; y--) {
        for (let x = 0; x < width; x++) {
            const i = (y * width + x) * 4;
            view.setUint8(offset++, data[i + 2]); // B
            view.setUint8(offset++, data[i + 1]); // G
            view.setUint8(offset++, data[i]);     // R
        }
        for (let p = 0; p < padding; p++) {
            view.setUint8(offset++, 0);
        }
    }

    return buffer;
}

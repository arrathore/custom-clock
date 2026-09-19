import { rgb565ToBytes } from "./imageUtils";

export async function sendImage(ip, pixels, width, height) {
    const bytes = rgb565ToBytes(pixels);

    console.log("pixels:", pixels.length);
    console.log("bytes:", bytes.length);

    const response = await fetch(`http://${ip}/image`, {
	method: "POST",
	headers: {
	    "Content-Type": "application/octet-stream",
	    "X-Image-Width": width,
	    "X-Image-Height": height,
	},
	body: bytes,
    });

    if (!response.ok)
	throw new Error(`ESP32 returned HTTP ${response.status}`);

    return response.text();
}


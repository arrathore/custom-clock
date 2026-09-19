import React from 'react';
import { DIGITS } from '../constants';
import { convertToBMP } from '../utils/imageUtils';

export default function ThemeAssetsGrid({ convertedTheme, themeHeight, digitHeight }) {
    if (!convertedTheme) return null;

    const scale = digitHeight / themeHeight;
    const scaledWidth = Math.round(themeHeight * scale * (convertedTheme.digits['0'].width / convertedTheme.digits['0'].height));

    const downloadBMP = (digit) => {
        const asset = convertedTheme.digits[digit];
        
        // Resize image
        const canvas = document.createElement('canvas');
        canvas.width = scaledWidth;
        canvas.height = digitHeight;
        const ctx = canvas.getContext('2d');
        ctx.drawImage(asset.image, 0, 0, scaledWidth, digitHeight);
        
        const imageData = ctx.getImageData(0, 0, scaledWidth, digitHeight);
        
        const buffer = convertToBMP(imageData);
        const blob = new Blob([buffer], { type: 'image/bmp' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `${digit}.bmp`;
        a.click();
        URL.revokeObjectURL(url);
    };

    return (
        <div className="theme-info">
            <h2>Theme Assets</h2>

            <div className="asset-grid">
                {DIGITS.map((digit) => {
                    const asset = convertedTheme.digits[digit];

                    return (
                        <div className="asset" key={digit}>
                            <img src={asset.previewUrl} alt={digit} />
                            <div>
                                <strong>{digit}.png</strong>
                                <span>
                                    {scaledWidth} × {digitHeight}
                                </span>
                                <span>
                                    {asset.rgb565.length * 2} bytes RGB565
                                </span>
                                <button onClick={() => downloadBMP(digit)}>Download BMP</button>
                            </div>
                        </div>
                    );
                })}
            </div>
        </div>
    );
}

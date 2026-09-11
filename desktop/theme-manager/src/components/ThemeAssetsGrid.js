import React from 'react';
import { DIGITS } from '../constants';

export default function ThemeAssetsGrid({ convertedTheme }) {
    if (!convertedTheme) return null;

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
                                    {asset.width} × {asset.height}
                                </span>
                                <span>
                                    {asset.rgb565.length * 2} bytes RGB565
                                </span>
                            </div>
                        </div>
                    );
                })}
            </div>
        </div>
    );
}

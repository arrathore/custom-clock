import React from 'react';

export default function ThemeDigit({ digit, digits, scale }) {
    const data = digits[digit];

    if (!data) return null;

    return (
        <img
            className="clock-digit"
            src={data.previewUrl}
            alt={digit}
            style={{
                height: `${data.height * scale}px`,
                width: `${data.width * scale}px`,
            }}
        />
    );
}

import React, { useState } from 'react';
import { DIGITS } from '../constants';
import { sendImage } from '../utils/esp32';
import { convertToRGB565 } from '../utils/imageUtils';

export default function ESP32Sender({ convertedTheme, themeHeight, digitHeight, backgroundColor }) {
    const [ip, setIp] = useState('');
    const [selectedDigit, setSelectedDigit] = useState('0');
    const [status, setStatus] = useState(null);
    const [sending, setSending] = useState(false);

    if (!convertedTheme) return null;

    const scale = digitHeight / themeHeight;
    const scaledWidth = Math.round(themeHeight * scale * (convertedTheme.digits['0'].width / convertedTheme.digits['0'].height));

    const handleSend = async (e) => {
        e.preventDefault();
        if (!ip.trim()) {
            setStatus({ type: 'error', message: 'Please enter an ESP32 IP address.' });
            return;
        }

        const asset = convertedTheme.digits[selectedDigit];
        
        setSending(true);
        setStatus(null);

        try {
            const canvas = document.createElement('canvas');
            canvas.width = scaledWidth;
            canvas.height = digitHeight;
            const ctx = canvas.getContext('2d');
            ctx.drawImage(asset.image, 0, 0, scaledWidth, digitHeight);
            
            const imageData = ctx.getImageData(0, 0, scaledWidth, digitHeight);
            const { rgb565 } = convertToRGB565(imageData, backgroundColor);
            
            const result = await sendImage(ip.trim(), rgb565, scaledWidth, digitHeight);
            setStatus({ type: 'success', message: `Successfully sent! Response: ${result || 'OK'}` });
        } catch (err) {
            setStatus({ type: 'error', message: `Failed to send: ${err.message}` });
        } finally {
            setSending(false);
        }
    };

    return (
        <div className="control-section esp32-sender">
            <h2>Send single image to ESP32</h2>
            <form onSubmit={handleSend}>
                <label>
                    ESP32 IP Address
                    <input
                        type="text"
                        placeholder="e.g. 192.168.1.50"
                        value={ip}
                        onChange={(e) => setIp(e.target.value)}
                    />
                </label>

                <label>
                    Select Asset
                    <select
                        value={selectedDigit}
                        onChange={(e) => setSelectedDigit(e.target.value)}
                    >
                        {DIGITS.map((digit) => (
                            <option key={digit} value={digit}>
                                Digit {digit}.png ({convertedTheme.digits[digit].width}×{convertedTheme.digits[digit].height})
                            </option>
                        ))}
                    </select>
                </label>

                <button type="submit" disabled={sending}>
                    {sending ? 'Sending...' : 'Send RGB565 Stream'}
                </button>
            </form>

            {status && (
                <div className={status.type === 'error' ? 'error' : 'success'}>
                    {status.message}
                </div>
            )}
        </div>
    );
}

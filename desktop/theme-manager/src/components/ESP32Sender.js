import React, { useState } from 'react';
import { DIGITS } from '../constants';
import { sendImage, sendTheme } from '../utils/esp32';
import { convertToRGB565 } from '../utils/imageUtils';

export default function ESP32Sender({ convertedTheme, themeHeight, digitHeight, backgroundColor }) {
    const [ip, setIp] = useState('');
    const [selectedDigit, setSelectedDigit] = useState('0');
    const [status, setStatus] = useState(null);
    const [sending, setSending] = useState(false);
    const [sendingTheme, setSendingTheme] = useState(false);

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
            setStatus({ type: 'success', message: `Successfully sent image! Response: ${result || 'OK'}` });
        } catch (err) {
            setStatus({ type: 'error', message: `Failed to send image: ${err.message}` });
        } finally {
            setSending(false);
        }
    };

    const handleSendTheme = async (e) => {
        e.preventDefault();
        if (!ip.trim()) {
            setStatus({ type: 'error', message: 'Please enter an ESP32 IP address.' });
            return;
        }

        setSendingTheme(true);
        setStatus(null);

        try {
            // Process all digits to scaledWidth x digitHeight just like single image or preview
            const processedDigits = {};
            for (const digit of DIGITS) {
                const asset = convertedTheme.digits[digit];
                const canvas = document.createElement('canvas');
                canvas.width = scaledWidth;
                canvas.height = digitHeight;
                const ctx = canvas.getContext('2d');
                ctx.drawImage(asset.image, 0, 0, scaledWidth, digitHeight);
                
                const imageData = ctx.getImageData(0, 0, scaledWidth, digitHeight);
                const { rgb565 } = convertToRGB565(imageData, backgroundColor);

                processedDigits[digit] = {
                    ...asset,
                    rgb565,
                };
            }

            const processedTheme = {
                ...convertedTheme,
                digits: processedDigits,
            };

            await sendTheme(ip.trim(), processedTheme, scaledWidth, digitHeight);
            setStatus({ type: 'success', message: 'Successfully sent entire theme!' });
        } catch (err) {
            setStatus({ type: 'error', message: `Failed to send theme: ${err.message}` });
        } finally {
            setSendingTheme(false);
        }
    };

    return (
        <div className="control-section esp32-sender">
            <h2>Send to ESP32</h2>
            
            <div style={{ marginBottom: '20px' }}>
                <label>
                    ESP32 IP Address
                    <input
                        type="text"
                        placeholder="e.g. 192.168.1.50"
                        value={ip}
                        onChange={(e) => setIp(e.target.value)}
                    />
                </label>
            </div>

            <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '20px' }}>
                <form onSubmit={handleSend}>
                    <h3>Send Single Image</h3>
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

                    <button type="submit" disabled={sending || sendingTheme}>
                        {sending ? 'Sending...' : 'Send RGB565 Stream'}
                    </button>
                </form>

                <form onSubmit={handleSendTheme}>
                    <h3>Send Entire Theme</h3>
                    <p style={{ fontSize: '0.9em', color: '#666', margin: '10px 0' }}>
                        Sends all 10 digits ({scaledWidth}×{digitHeight}) to the ESP32 theme endpoint.
                    </p>

                    <button type="submit" disabled={sending || sendingTheme}>
                        {sendingTheme ? 'Sending Theme...' : 'Send Entire Theme'}
                    </button>
                </form>
            </div>

            {status && (
                <div className={status.type === 'error' ? 'error' : 'success'} style={{ marginTop: '15px' }}>
                    {status.message}
                </div>
            )}
        </div>
    );
}

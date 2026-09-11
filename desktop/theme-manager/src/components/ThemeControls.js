import React from 'react';

export default function ThemeControls({
    theme,
    error,
    displayWidth,
    displayHeight,
    digitHeight,
    spacing,
    backgroundColor,
    timeString,
    use24Hour,
    onThemeFolder,
    setDisplayWidth,
    setDisplayHeight,
    setDigitHeight,
    setSpacing,
    setBackgroundColor,
    setTimeString,
    setUse24Hour,
}) {
    return (
        <section className="controls">
            <div className="control-section">
                <h2>Theme</h2>

                <label className="folder-button">
                    Choose Theme Folder
                    <input
                        type="file"
                        webkitdirectory=""
                        directory=""
                        multiple
                        accept=".png,image/png"
                        onChange={onThemeFolder}
                    />
                </label>

                <p className="help">
                    The folder must contain 0.png through 9.png
                </p>

                {error && <div className="error">{error}</div>}
            </div>

            <div className="control-section">
                <h2>Display</h2>

                <div className="control-row">
                    <label>
                        Width
                        <input
                            type="number"
                            min="1"
                            value={displayWidth}
                            onChange={(event) =>
                                setDisplayWidth(Number(event.target.value))
                            }
                        />
                    </label>

                    <label>
                        Height
                        <input
                            type="number"
                            min="1"
                            value={displayHeight}
                            onChange={(event) =>
                                setDisplayHeight(Number(event.target.value))
                            }
                        />
                    </label>
                </div>

                <label>
                    Background color
                    <input
                        type="color"
                        value={backgroundColor}
                        onChange={(event) =>
                            setBackgroundColor(event.target.value)
                        }
                    />
                    <span>{backgroundColor}</span>
                </label>

                <label>
                    Digit height
                    <input
                        type="range"
                        min="20"
                        max={Math.max(displayHeight, 20)}
                        value={digitHeight}
                        onChange={(event) =>
                            setDigitHeight(Number(event.target.value))
                        }
                    />
                    <span>{digitHeight}px</span>
                </label>

                <label>
                    Time
                    <input
                        type="time"
                        value={timeString}
                        onChange={(event) => setTimeString(event.target.value)}
                    />
                </label>

                <label className="toggle-label">
                    <input
                        type="checkbox"
                        checked={use24Hour}
                        onChange={(event) => setUse24Hour(event.target.checked)}
                    />
                    24-hour time
                </label>

                <label>
                    Digit spacing
                    <input
                        type="range"
                        min="0"
                        max="30"
                        value={spacing}
                        onChange={(event) =>
                            setSpacing(Number(event.target.value))
                        }
                    />
                    <span>{spacing}px</span>
                </label>
            </div>

            {theme && (
                <div className="control-section">
                    <h2>Conversion</h2>

                    <div className="format-info">
                        <strong>RGB565</strong>
                        <span>16 bits per pixel</span>
                    </div>

                    <p className="help">
                        The PNGs have been converted to RGB565 in memory.
                    </p>
                </div>
            )}
        </section>
    );
}

import ThemeDigit from './ThemeDigit';
import ThemeAssetsGrid from './ThemeAssetsGrid';
import { formatTime } from '../utils/timeUtils';
import { getContrastingColor } from '../utils/imageUtils';

export default function ThemePreview({
    theme,
    convertedTheme,
    displayWidth,
    displayHeight,
    backgroundColor,
    timeString,
    use24Hour,
    spacing,
    scale,
    digitHeight,
}) {
    return (
        <section className="preview-section">
            <h2>Preview</h2>

            <div className="preview-wrapper">
                <div
                    className="clock-preview"
                    style={{
                        width: `${displayWidth}px`,
                        height: `${displayHeight}px`,
                        backgroundColor,
                    }}
                >
                    {theme ? (
                        <div
                            className="clock-digits"
                            style={{
                                gap: `${spacing}px`,
                            }}
                        >
                            {formatTime(timeString, use24Hour).split("").map((character, index) => {
                                if (character === ":") {
                                    return (
                                        <span
                                            className="clock-colon"
                                            key={`${character}-${index}`}
                                            style={{
                                                color: getContrastingColor(backgroundColor),
                                            }}
                                        >
                                            :
                                        </span>
                                    );
                                }

                                return (
                                    <ThemeDigit
                                        key={`${character}-${index}`}
                                        digit={character}
                                        digits={convertedTheme.digits}
                                        scale={scale}
                                    />
                                );
                            })}
                        </div>
                    ) : (
                        <div className="empty-preview">
                            Choose a theme folder to preview it.
                        </div>
                    )}
                </div>
            </div>

            {theme && (
                <ThemeAssetsGrid
                    convertedTheme={convertedTheme}
                    themeHeight={theme.height}
                    digitHeight={digitHeight}
                />
            )}
        </section>
    );
}

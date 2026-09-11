import React, { useEffect, useMemo, useState } from 'react';
import './App.css';

const DIGITS = ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"];

const DEFAULT_DISPLAY = {
    width: 320,
    height: 240,
};

function loadImage(file) {
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
function rgbTo565(r, g, b) {
    const red = (r >> 3) & 0x1f;
    const green = (g >> 2) & 0x3f;
    const blue = (b >> 3) & 0x1f;

    return (red << 11) | (green << 5) | blue;
}

// convert image to RGB565 pixels and return both pixels and the converted preview data URL
function convertToRGB565(imageData) {
    const { data, width, height } = imageData;
    const pixels = new Uint16Array(data.length / 4);

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
        const a = data[i + 3];

        const val = rgbTo565(r, g, b);
        pixels[pixel] = val;

        // unpack RGB565 back to 8-bit channels to preview exact converted appearance
        const r5 = (val >> 11) & 0x1f;
        const g6 = (val >> 5) & 0x3f;
        const b5 = val & 0x1f;

        outData[i] = Math.round((r5 * 255) / 31);
        outData[i + 1] = Math.round((g6 * 255) / 63);
        outData[i + 2] = Math.round((b5 * 255) / 31);
        outData[i + 3] = a; // preserve original alpha channel
    }

    context.putImageData(outputImageData, 0, 0);
    const previewUrl = canvas.toDataURL("image/png");

    return {
        rgb565: pixels,
        previewUrl,
    };
}

function ThemeDigit({ digit, digits, scale }) {
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

function formatTime(time, use24Hour) {
  if (use24Hour) {
    return time;
  }

  const [hours, minutes] = time.split(":");
  const hour = Number(hours);

  const displayHour = hour % 12 || 12;
  const period = hour < 12 ? "AM" : "PM";

  return `${String(displayHour).padStart(2, "0")}:${minutes} ${period}`;
}
	
function App() {
  const [theme, setTheme] = useState(null);
  const [error, setError] = useState(null);
  
  const [displayWidth, setDisplayWidth] = useState(DEFAULT_DISPLAY.width);
  const [displayHeight, setDisplayHeight] = useState(DEFAULT_DISPLAY.height);
  
  const [digitHeight, setDigitHeight] = useState(120);
  const [spacing, setSpacing] = useState(4);

  const [timeString, setTimeString] = useState(() => {
    const now = new Date();
    const hours = String(now.getHours()).padStart(2, '0');
    const minutes = String(now.getMinutes()).padStart(2, '0');
    return `${hours}:${minutes}`;
  });

  const [use24Hour, setUse24Hour] = useState(true);

  // determine scale from height of theme's digits
  const scale = useMemo(() => {
    if (!theme || theme.height === 0) {
      return 1;
    }
    
    return digitHeight / theme.height;
  }, [theme, digitHeight]);
  
  async function handleThemeFolder(event) {
                setError(null);
                setTheme(null);

                const files = Array.from(event.target.files);

                // ignore everything except PNG files that match our format
                const digitFiles = {};

        for (const file of files) {
            const match = file.name.match(/^([0-9])\.png$/i);

            if (match) {
                digitFiles[match[1]] = file;
            }
        }

        // detect missing digits
        const missing = DIGITS.filter((digit) => !digitFiles[digit]);
        if (missing.length > 0) {
            setError(
                `Theme is missing: ${missing
                    .map((digit) => `${digit}.png`)
                    .join(", ")}`
            );
            return;
        }

        try {
            const loadedTheme = {};

            for (const digit of DIGITS) {
                const file = digitFiles[digit];
                const loaded = await loadImage(file);

                // create canvas to access raw pixels and perform RGB565 conversion
                const canvas = document.createElement("canvas");
                canvas.width = loaded.width;
                canvas.height = loaded.height;

                const context = canvas.getContext("2d", {
                    willReadFrequently: true,
                });

                context.drawImage(loaded.image, 0, 0);

                const imageData = context.getImageData(
                    0,
                    0,
                    loaded.width,
                    loaded.height
                );

                const { rgb565, previewUrl } = convertToRGB565(imageData);

                loadedTheme[digit] = {
                    file,
                    image: loaded.image,
                    url: URL.createObjectURL(file),
                    previewUrl,
                    width: loaded.width,
                    height: loaded.height,
                    rgb565,
                };
            }

            // enforce consistent height across theme digits
            const firstHeight = loadedTheme["0"].height;
            const inconsistentHeight = DIGITS.find(
                (digit) => loadedTheme[digit].height !== firstHeight
            );

            if (inconsistentHeight !== undefined) {
                setError(
                    `Digit heights are inconsistent. 0.png is ${firstHeight}px high, ` +
                        `${inconsistentHeight}.png is ${loadedTheme[inconsistentHeight].height}px high.`
                );
                return;
            }

            setTheme({
                digits: loadedTheme,
                height: firstHeight,
            });

            setDigitHeight(Math.min(firstHeight * 2, 160));
        } catch (err) {
            setError(err.message);
        }
    }

    // clean up object URLs when theme is replaced/unmounted
    useEffect(() => {
        return () => {
            if (theme) {
                Object.values(theme.digits).forEach((digit) => {
                    URL.revokeObjectURL(digit.url);
                });
            }
        };
    }, [theme]);

    return (
    <div className="app">
      <header>
        <h1>Theme Manager</h1>
        <p>
          Load a set of digit images and see how they might look on
          the clock.
        </p>
      </header>

      <main>
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
                onChange={handleThemeFolder}
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
                  value= {displayHeight}
                  onChange={(event) =>
                setDisplayHeight(Number(event.target.value))
                }
                />
              </label>
            </div>

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
              <span>
                16 bits per pixel
              </span>
            </div>

            <p className="help">
              The PNGs have been converted to RGB565 in memory.
            </p>
          </div>
          )}
        </section>

        <section className="preview-section">
          <h2>Preview</h2>

          <div className="preview-wrapper">
            <div
              className="clock-preview"
              style={{
              width: `${displayWidth}px`,
              height: `${displayHeight}px`,
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
                  >
                  :
                </span>
                );
                }

                return (
                <ThemeDigit
                  key={`${character}-${index}`}
                  digit={character}
                  digits={theme.digits}
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
          <div className="theme-info">
            <h2>Theme Assets</h2>

            <div className="asset-grid">
              {DIGITS.map((digit) => {
              const asset = theme.digits[digit];

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
          )}
        </section>
      </main>
    </div>
    );
}

export default App;

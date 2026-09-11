import React, { useEffect, useMemo, useState } from 'react';
import { DIGITS, DEFAULT_DISPLAY } from './constants';
import { loadImage, convertToRGB565 } from './utils/imageUtils';
import ThemeControls from './components/ThemeControls';
import ThemePreview from './components/ThemePreview';
import './App.css';

function App() {
  const [theme, setTheme] = useState(null);
  const [error, setError] = useState(null);
  
  const [displayWidth, setDisplayWidth] = useState(DEFAULT_DISPLAY.width);
  const [displayHeight, setDisplayHeight] = useState(DEFAULT_DISPLAY.height);
  
  const [digitHeight, setDigitHeight] = useState(120);
  const [spacing, setSpacing] = useState(4);

  const [backgroundColor, setBackgroundColor] = useState("#000000");

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
	
        loadedTheme[digit] = {
          file,
          image: loaded.image,
          url: URL.createObjectURL(file),
          imageData,
          width: loaded.width,
          height: loaded.height,
          rgb565: null,
	        previewUrl: null,
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

  const convertedTheme = useMemo(() => {
    if (!theme) return null;

    const digits = {};

    for (const digit of DIGITS) {
      const asset = theme.digits[digit];

      const { rgb565, previewUrl } = convertToRGB565(
        asset.imageData,
        backgroundColor
      );

      digits[digit] = {...asset, rgb565, previewUrl};
    }

    return {...theme, digits};
  }, [theme, backgroundColor]);
  
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
        <ThemeControls
          theme={theme}
          error={error}
          displayWidth={displayWidth}
          displayHeight={displayHeight}
          digitHeight={digitHeight}
          spacing={spacing}
          backgroundColor={backgroundColor}
          timeString={timeString}
          use24Hour={use24Hour}
          onThemeFolder={handleThemeFolder}
          setDisplayWidth={setDisplayWidth}
          setDisplayHeight={setDisplayHeight}
          setDigitHeight={setDigitHeight}
          setSpacing={setSpacing}
          setBackgroundColor={setBackgroundColor}
          setTimeString={setTimeString}
          setUse24Hour={setUse24Hour}
        />

        <ThemePreview
          theme={theme}
          convertedTheme={convertedTheme}
          displayWidth={displayWidth}
          displayHeight={displayHeight}
          backgroundColor={backgroundColor}
          timeString={timeString}
          use24Hour={use24Hour}
          spacing={spacing}
          scale={scale}
        />
      </main>
    </div>
  );
}

export default App;

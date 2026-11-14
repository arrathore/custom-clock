# resolve repository root from this script's location
ROOT_DIR="$(cd "$(dirname "$BASH_SOURCE[0]}")/.." && pwd)"

FRONTEND_DIR="$ROOT_DIR/theme-manager"
BUILD_DIR="$ROOT_DIR/theme-manager/build"
DEST_DIR="$ROOT_DIR/esp32/clock-firmware/web/www"

echo "Building frontend..."

cd "$FRONTEND_DIR"
GENERATE_SOURCEMAP=false npm run build

echo "Copying frontend build to firmware..."

mkdir -p "$DEST_DIR"
rm -rf "$DEST_DIR"/*
cp -R "$BUILD_DIR"/. "$DEST_DIR"/

echo "Frontend build copied successfully."
echo "Destination: $DEST_DIR"


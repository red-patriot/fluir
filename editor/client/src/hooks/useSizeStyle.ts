import { Location } from '../models/fluir_module';

export const ZOOM_SCALAR = 10;
export const HEADER_HEIGHT = 4;

export const getSizeStyle = (location: Location) => {
  const { width, height } = location;
  return {
    width: `${width * ZOOM_SCALAR}px`,
    height: `${height * ZOOM_SCALAR}px`,
  };
};

export const getLocationStyle = (location: Location) => {
  const { x, y, z } = location;
  return {
    left: `${x * ZOOM_SCALAR}px`,
    top: `${y * ZOOM_SCALAR}px`,
    zIndex: `${z}`,
  };
};

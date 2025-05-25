import { Location } from '../models/fluir_module';
import { useAppSelector } from '../store';

export const ZOOM_SCALAR = 0.3;

export const getSizeStyle = (location: Location) => {
  const zoom = useAppSelector((state) => state.program.zoom) * ZOOM_SCALAR;
  const { width, height } = location;
  return { width: `${width}px`, height: `${height}px` };
};

export const getLocationStyle = (location: Location) => {
  const zoom = useAppSelector((state) => state.program.zoom) * ZOOM_SCALAR;
  const { x, y, z } = location;
  return {
    left: `${x}px`,
    top: `${y}px`,
    zIndex: `${z}`,
  };
};

export function getFontSize(unscaled: Location | number) {
  const unscaledSize =
    typeof unscaled === 'number' ? unscaled : (unscaled as Location).height * 2;

  return { fontSize: unscaledSize * ZOOM_SCALAR };
}

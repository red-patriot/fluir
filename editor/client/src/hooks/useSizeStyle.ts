import { Location } from '../models/fluir_module';
import { useAppSelector } from '../store';

export const ZOOM_SCALAR = 0.3;

export const getSizeStyle = (location: Location) => {
  const zoom = useAppSelector((state) => state.program.zoom) * ZOOM_SCALAR;
  const { width, height } = location;
  return { width: `${width * zoom}px`, height: `${height * zoom}px` };
};

export const getLocationStyle = (location: Location) => {
  const zoom = useAppSelector((state) => state.program.zoom) * ZOOM_SCALAR;
  const { x, y, z } = location;
  return {
    left: `${x * zoom}px`,
    top: `${y * zoom}px`,
    zIndex: `${z}`,
  };
};

export function getFontSize(unscaled: Location | number) {
  const zoom = useAppSelector((state) => state.program.zoom) * ZOOM_SCALAR;
  const unscaledSize =
    typeof unscaled === 'number' ? unscaled : (unscaled as Location).height * 2;

  return { fontSize: unscaledSize * zoom * ZOOM_SCALAR };
}

import { useCallback } from 'react';
import { Location } from '../models/fluir_module';
import { useAppSelector } from '../store';

export const ZOOM_SCALAR = 0.3;

export function useSizeStyle(location: Location) {
  const doGetSize = () => {
    const zoom = useAppSelector((state) => state.program.zoom) * ZOOM_SCALAR;
    const { x, y, z, width, height } = location;
    return {
      left: `${x * zoom}px`,
      top: `${y * zoom}px`,
      width: `${width * zoom}px`,
      height: `${height * zoom}px`,
      zIndex: `${z}`,
    };
  };

  const doGetFontSize = (unscaledSize: number = location.height * 2) => {
    const zoom = useAppSelector((state) => state.program.zoom) * ZOOM_SCALAR;

    return { fontSize: unscaledSize * zoom * ZOOM_SCALAR };
  };

  const getSizeStyle = useCallback(doGetSize, []);
  const getFontSize = useCallback(doGetFontSize, []);

  return { getSizeStyle, getFontSize };
}

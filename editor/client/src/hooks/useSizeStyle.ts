import { useCallback } from 'react';
import { Location } from '../models/fluir_module';
import { useAppSelector } from '../store';

export const ZOOM_SCALAR = 1.5;

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

  const getSizeStyle = useCallback(doGetSize, []);

  return getSizeStyle;
}

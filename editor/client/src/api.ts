// TODO: Get the path from config
const BASE_URL =
  window.location.protocol + '//' + window.location.hostname + ':8001';

export const SERVER_API = {
  newProgram: BASE_URL + '/api/module/new/',
  openProgram: BASE_URL + '/api/module/open/',
  editProgram: BASE_URL + '/api/module/edit/',
  undo: BASE_URL + '/api/module/undo/',
  redo: BASE_URL + '/api/module/redo/',
  saveAs: BASE_URL + '/api/module/save/',
};

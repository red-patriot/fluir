// TODO: Get the path from config
const BASE_URL =
  window.location.protocol + '//' + window.location.hostname + ':8001';

export const SERVER_API = {
  openProgram: BASE_URL + '/api/module/open/',
  editProgram: BASE_URL + '/api/module/edit/',
  saveAs: BASE_URL + '/api/module/save/',
};

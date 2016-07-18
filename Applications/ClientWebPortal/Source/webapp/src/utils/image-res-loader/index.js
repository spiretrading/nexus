import deviceDetector from 'utils/device-detector';

/** Helper class for resolution dependent images */
class ImageResLoader {
  getResPath(imagePath) {
    if (deviceDetector.isHighDefinitionDisplay()){
      let lastBackslashIndex = imagePath.lastIndexOf("/");
      let folderPath = imagePath.substring(0, lastBackslashIndex + 1);
      let imageName = imagePath.substring(lastBackslashIndex + 1);
      let periodIndex = imageName.indexOf(".");
      let fileName = imageName.substring(0, periodIndex);
      let extension = imageName.substring(periodIndex + 1);
      let newFileName = fileName + "@2x." + extension;
      return folderPath + newFileName;
    } else {
      return imagePath;
    }
  }
}

export default new ImageResLoader();
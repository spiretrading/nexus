class TimeFormatter {
  formatDuration(time) {
    let colonStrippedTime = time.replace(/:/g, '');
    colonStrippedTime = colonStrippedTime.replace(/;/g, '');
    if (colonStrippedTime.length > 6) {
      colonStrippedTime = colonStrippedTime.substring(0, 6);
    }
    let numZerosNeeded = 6 - colonStrippedTime.length;
    if (numZerosNeeded > 0) {
      for (let i=0; i<numZerosNeeded; i++) {
        colonStrippedTime = '0' + colonStrippedTime;
      }
    }
    let formattedTime = colonStrippedTime.replace(/\B(?=(\d{2})+(?!\d))/g, ":");
    return formattedTime.substring(0, 8);
  }
}

export default new TimeFormatter();

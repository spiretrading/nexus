class TimeFormatter {
  formatDuration(time) {
    let colonStrippedTime = time.replace(':', '');
    colonStrippedTime = colonStrippedTime.replace(';', '');
    let numZerosNeeded = 6 - colonStrippedTime.length;
    if (numZerosNeeded > 0) {
      for (let i=0; i<numZerosNeeded; i++) {
        colonStrippedTime = '0' + colonStrippedTime;
      }
    }
    let formattedTime = colonStrippedTime.replace(/\B(?=(\d{2})+(?!\d))/g, ":");
    return formattedTime;
  }
}

export default new TimeFormatter();

import csvParser from 'papaparse';

export default class {
  constructor(sourceModel) {
    this.sourceModel = sourceModel;
  }

  getCsvString() {
    let rowCount = this.sourceModel.getRowCount();
    let columnCount = this.sourceModel.getColumnCount();

    // headers
    let headers = [];
    for (let i=0; i<columnCount; i++) {
      headers.push(this.sourceModel.getColumnHeader(i));
    }

    // data
    let data = [];
    for (let i=0; i<rowCount; i++) {
      let rowData = [];
      for (let j=0; j<columnCount; j++) {
        let value = this.sourceModel.getValueAt(j, i);
        rowData.push(value);
      }
      data.push(rowData);
    }

    return csvParser.unparse({
      fields: headers,
      data: data
    });
  }
}

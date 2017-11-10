import ViewModel from '../../../../webapp/utils/table-models/view-model';

describe('ViewModel', function() {
  beforeAll(function() {
    let sourceModel = new MockSourceModel();
    this.viewModel = new ViewModel(sourceModel);
  });

  it('getRowCount', function() {
    expect(this.viewModel.getRowCount()).toBe(2);
  });

  it('getColumnCount', function() {
    expect(this.viewModel.getColumnCount()).toBe(3);
  });

  it('getColumnName', function() {
    expect(this.viewModel.getColumnName(2)).toBe('Name');
  });

  describe('getValueAt', function(){
    it('Number - even number row', function() {
      let value = this.viewModel.getValueAt(0,0);
      expect(value.value).toBe(12345678910);
      expect(value.display).toBe('12,345,678,910');
    });

    it('String - even number row', function() {
      let value = this.viewModel.getValueAt(1,0);
      expect(value.value).toBe('Some String Value 1');
      expect(value.display).toBe('Some String Value 1');
    });

    it('All others', function() {
      let value = this.viewModel.getValueAt(2,0);
      expect(value.value.name).toBe('John Doe');
      expect(value.display).toBe('John Doe');
    });
  });
});

class MockSourceModel {
  constructor() {
    this.columnNames = [
      'Number',
      'String',
      'Name'
    ];

    this.data = [
      [
        12345678910,
        'Some String Value 1',
        new MockName('John Doe')
      ],
      [
        5123478,
        'Some String Value 2',
        new MockName('Will Smith')
      ]
    ];
  }

  getRowCount() {
    return this.data.length;
  }

  getColumnCount() {
    return this.columnNames.length;
  }

  getColumnName(x) {
    return this.columnNames[x];
  }

  getValueAt(x, y) {
    return this.data[y][x];
  }
}

class MockName {
  constructor(name) {
    this.name = name;
  }

  toString() {
    return this.name;
  }
}

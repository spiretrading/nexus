import ArrayStringKeyGenerator from '../../../webapp/utils/array-string-key-generator';

describe('ArrayStringKeyGenerator', function() {
  beforeAll(function() {
    this.keyGenerator = new ArrayStringKeyGenerator();
  });

  describe('get', function() {
    it('Value containing a null', function() {
      let values = [1, null, 3];
      let testMethod = function() {
        this.keyGenerator.get(values);
      }.bind(this);
      let errorMessage = 'Values must not contain a null';
      expect(testMethod).toThrow(new RangeError(errorMessage));
    });

    it('Valid values', function() {
      let objectWithToString = {
        name: 'Tester',
        toString: function() {
          return this.name;
        }
      };
      let values = [1, 'Some String', objectWithToString, 4];
      let key = this.keyGenerator.get(values);
      expect(key).toBe('(1)(Some String)(Tester)(4)');
    });
  })
});

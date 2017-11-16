import SignalManager from '../../../webapp/utils/signal-manager';

describe('SignalManager', function() {
  describe('addListener', function() {
    let signalManager;

    beforeEach(function() {
      signalManager = new SignalManager();
    });

    it('Non function listener', function() {
      let testMethod = function() {
        signalManager.addListener(2);
      };
      let errorMessage = 'Listener must be a function';
      expect(testMethod).toThrow(new RangeError(errorMessage));
    });
  })

  it('Valid listener add and emit', function(done) {
    let signalManager = new SignalManager();
    let listener = function(signalType, payload) {
      expect(signalType).toBe(2);
      expect(payload).toBe('test payload');
      done();
    };
    let firstSubId = signalManager.addListener(listener);
    let secondSubId = signalManager.addListener(function(){});
    expect(secondSubId - firstSubId).toBe(1);
    signalManager.emitSignal(2, 'test payload');
  });

  describe('removeListener', function() {
    let signalManager;

    beforeEach(function() {
      signalManager = new SignalManager();
    });

    it('Non existent subscription ID', function() {
      signalManager.addListener(function() {});
      let testMethod = function() {
        let invalidSubId = 100;
        signalManager.removeListener(invalidSubId);
      };
      let errorMessage = 'Subscription ID does not exist';
      expect(testMethod).toThrow(new RangeError(errorMessage));
    });

    it('Valid subscription ID', function() {
      let subId = signalManager.addListener(function() {});
      signalManager.removeListener(subId);
    });
  })
});

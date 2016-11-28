/** Class applies various input validation related logic to UI components */
class InputValidator {
  onlyNumbers($input) {
    $input.keydown((e) => {
      // Allow: backspace, delete, tab, escape, enter and .
      if ($.inArray(e.keyCode, [46, 8, 9, 27, 13, 110, 190]) !== -1 ||
        // Allow: Ctrl+A, Command+A
        (e.keyCode === 65 && (e.ctrlKey === true || e.metaKey === true)) ||
        // Allow: home, end, left, right, down, up
        (e.keyCode >= 35 && e.keyCode <= 40)) {
        // let it happen, don't do anything
        return;
      }
      // Ensure that it is a number and stop the keypress
      if ((e.shiftKey || (e.keyCode < 48 || e.keyCode > 57)) && (e.keyCode < 96 || e.keyCode > 105)) {
        e.preventDefault();
      }
    });
  }

  onlyAmPm($input) {
    $input.keydown((event) => {
      if ($.inArray(event.keyCode, [38, 40]) === -1){
        event.preventDefault();
      } else {
        let $ampmInput = $(event.currentTarget);
        let currentValue = $ampmInput.val();

        if (currentValue == 'AM') {
          $ampmInput.val('PM');
        } else {
          $ampmInput.val('AM');
        }
      }
    });
  }
}

export default new InputValidator();

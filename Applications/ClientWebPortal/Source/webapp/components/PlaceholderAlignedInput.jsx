define(['react', 'react-dom', 'jquery'],
  function(React, ReactDOM, $) {

    /** A React Component representing a centered input field with a placeholder
     *  value.
     */
    class PlaceholderAlignedInput extends React.Component {

      /**
       * Constructs a PlaceholderAlignedInput.
       */
      constructor() {
        super();
        this.state =
          {
            value: ''
          };
        this.target = null;
        this.hasUserInput = this.hasUserInput.bind(this);
        this.preventCursorAdjustment = this.preventCursorAdjustment.bind(this);
        this.handleChange = this.handleChange.bind(this);
        this.handleKeyDown = this.handleKeyDown.bind(this);
        this.handleTouchStart = this.handleTouchStart.bind(this);
      }

      /**
       * Returns true if this component has a non-empty value.
       */
      hasUserInput() {
        return this.state.value != '';
      }

      render() {
        return (
          <input
            {...this.props}
            placeholder = ''
            ref = {
              function(ref) {
                this.target = ref;
              }.bind(this)
            }
            type = 'text'
            onChange = {this.handleChange}
            onMouseDown = {this.preventCursorAdjustment}
            onKeyDown = {this.handleKeyDown}
            onDragStart = {this.preventCursorAdjustment}
            onTouchStart = {this.handleTouchStart}
            value = {
              function() {
                if(this.hasUserInput()) {
                  return this.state.value;
                } else {
                  return this.props.placeholder;
                }
              }.bind(this)()
            }
          />
        );
      }

      componentDidUpdate() {
        if(!this.hasUserInput() &&
            this.target == this.target.ownerDocument.activeElement) {
          this.target.setSelectionRange(0, 0);
        }
      }

      /** @private */
      preventCursorAdjustment(event) {
        if(this.hasUserInput()) {
          return;
        }
        event.stopPropagation();
        event.preventDefault();
        if(!this.target.hasFocus) {
          this.target.focus();
        }
        return false;
      }

      /** @private */
      handleChange(event) {
        var updatedValue = null;
        if(event.target.value != '') {
          if(!this.hasUserInput()) {
            updatedValue = event.target.value.substr(
              event.target.selectionStart - 1, 1);
          } else {
            updatedValue = event.target.value;
          }
        } else {
          updatedValue = '';
        }
        var previousState = this.hasUserInput();
        this.setState({value : updatedValue},
          function() {
            if(previousState != this.hasUserInput()) {
              if(this.props.type == 'password') {
                if(this.hasUserInput()) {
                  $(this.target).prop('type', 'password');
                  this.target.setSelectionRange(1, 1);
                } else {
                  $(this.target).prop('type', 'text');
                }
              }
            }
            if(this.props.onChange != null) {
              return this.props.onChange(event);
            }
          }.bind(this));
      }

      /** @private */
      handleKeyDown(event) {
        if(event.keyCode >= 37 && event.keyCode <= 40 ||
            event.keyCode == 8 || event.keyCode == 46) {
          if(!this.preventCursorAdjustment(event)) {
            return false;
          }
        }
        if(this.props.onKeyDown != null) {
          return this.props.onKeyDown(event);
        }
      }

      /** @private */
      handleTouchStart(event) {
        if(!this.hasUserInput()) {
          this.preventCursorAdjustment(event);
          this.target.blur();
          this.target.focus();
          return false;
        }
        if(this.props.onTouchStart != null) {
          return this.props.onTouchStart(event);
        }
      }
    }
  return PlaceholderAlignedInput;
});

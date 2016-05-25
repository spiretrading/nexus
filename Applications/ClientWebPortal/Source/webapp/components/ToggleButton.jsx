define(['radium', 'react'],
  function(Radium, React) {

    /** A React Component displaying a button that toggles between two
        states.
     */
    class ToggleButton extends React.Component {

      /**
       * Constructs a ToggleButton.
       * @param props The React props.
       */
      constructor(props) {
        super(props);
        this.state = {
          isToggled: this.props.isToggled
        };
        this.handleClick = this.handleClick.bind(this);
      }

      render() {
        var buttonStyle = {
          backgroundColor: 'transparent',
          border: 'none',
        };
        if(this.props.width != undefined && this.props.height != undefined) {
          buttonStyle['width'] = this.props.width;
          buttonStyle['height'] = this.props.height;
          buttonStyle['backgroundSize'] = this.props.width + ' ' +
            this.props.height;
        } else if(this.props.width != undefined) {
          buttonStyle['width'] = this.props.width;
        } else if(this.props.height != undefined) {
          buttonStyle['height'] = this.props.height;
        }
        if(this.state.isToggled) {
          buttonStyle['backgroundImage'] =
            'url("' + this.props.toggledImage + '")';
          if(this.props.toggledHighlightedImage != undefined) {
            buttonStyle[':hover'] = {
              backgroundImage:
                'url("' + this.props.toggledHighlightedImage + '")'
            }
          }
        } else {
          buttonStyle['backgroundImage'] =
            'url("' + this.props.baseImage + '")';
          if(this.props.baseHighlightedImage != undefined) {
            buttonStyle[':hover'] = {
              backgroundImage: 'url("' + this.props.baseHighlightedImage + '")'
            }
          }
        }
        return (
          <button
            type = "button"
            style = {buttonStyle}
            onClick = {this.handleClick} />);
      }

      /** @private */
      handleClick() {
        this.setState({isToggled: !this.state.isToggled},
          function() {
            this.props.onClick(this.state.isToggled);
          });
      }
    }
    ToggleButton.propTypes =
      {

        /** The path to the default image to display. */
        baseImage: React.PropTypes.string.isRequired,

        /** The path to the default image to display on hover. */
        baseHighlightedImage: React.PropTypes.string,

        /** The path to the toggled image to display. */
        toggledImage: React.PropTypes.string.isRequired,

        /** The path to the toggled image to display on hover. */
        toggledHighlightedImage: React.PropTypes.string,

        /** The width of the image. */
        width: React.PropTypes.object,

        /** The height of the image. */
        height: React.PropTypes.object,

        /** Whether the initial state of the button is toggled. */
        isToggled: React.PropTypes.bool,

        /** The onClick event handler. */
        onClick: React.PropTypes.func
      };
    ToggleButton.defaultProps =
      {
        isToggled: false,
        onClick: function(isToggled) {}
      };
    ToggleButton = Radium(ToggleButton);
    return ToggleButton;
  }
);

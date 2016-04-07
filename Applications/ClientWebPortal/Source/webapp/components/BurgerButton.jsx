define(['react', 'react-dom', 'jquery'],
  function(React, ReactDOM, $) {
    function drawStroke(context, x, y, width, height) {
      var radius = 3;
      context.beginPath();
      context.moveTo(x + radius, y);
      context.lineTo(x + width - radius, y);
      context.arcTo(x + width, y, x + width, y + radius, radius);
      context.lineTo(x + width, y + height - radius);
      context.arcTo(x + width, y + height, x + width - radius, y + height,
        radius);
      context.lineTo(x + radius, y + height);
      context.arcTo(x, y + height, x, y + height - radius, radius);
      context.lineTo(x, y + radius);
      context.arcTo(x, y, x + radius, y, radius);
      context.fill();
      context.closePath();
    }

    /** A React Component displaying a burger button. */
    class BurgerButton extends React.Component {

      /**
       * Constructs a BurgerButton.
       */
      constructor() {
        super();
        this.target = null;
      }

      render() {
        return (
          <div tabIndex = "0">
            <canvas
              ref = {
                function(ref) {
                  this.target = ref;
                }.bind(this)
              }
              onClick = {this.props.onClick}
              width = {this.props.width}
              height = {this.props.height}
            />
          </div>
        );
      }

      componentDidMount() {
        var context = this.target.getContext('2d');
        context.fillStyle = this.props.background_color;
        context.fillRect(0, 0, this.props.width, this.props.height);
        context.fillStyle = this.props.stroke_color;
        var gap = (6 * this.props.height) / 64;
        var startX = (10 * this.props.width) / 70;
        var width = (this.props.width - 2 * startX);
        var startY = (14 * this.props.height) / 64;
        var height = ((this.props.height - 2 * startY) / 3) - gap;
        for(var i = 0; i < 3; ++i) {
          drawStroke(context, startX, startY + gap / 2, width, height);
          startY += height + gap;
        }
      }
    }
    return BurgerButton;
  }
);

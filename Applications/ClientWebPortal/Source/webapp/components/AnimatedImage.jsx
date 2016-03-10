define(['react', 'react-dom'],
  function(React, ReactDOM) {

    /** A React Component containing an image that can be animated or
     *  stopped.
     */
    class AnimatedImage extends React.Component {
      render() {
        return (
          <img
            {...this.props}
            src = {
              function() {
                if(this.props.isPlaying) {
                  return this.props.animatedImage;
                } else {
                  return this.props.initialImage;
                }
              }.bind(this)()
            }
          />
        );
      }
    }
    AnimatedImage.propTypes =
      {

        /** The path to the initial image to display. */
        initialImage: React.PropTypes.string.isRequired,

        /** The path to the animated image to display. */
        animatedImage: React.PropTypes.string.isRequired,

        /** If true then the animated image is displayed, otherwise the
         *  initial image is displayed.
         */
        isPlaying: React.PropTypes.bool
      };
    AnimatedImage.defaultProps =
      {
        isPlaying: false
      };
    return AnimatedImage;
  });

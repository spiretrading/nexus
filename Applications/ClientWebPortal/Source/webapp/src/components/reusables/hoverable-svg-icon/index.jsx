import React, {Component} from 'react';
import imageResLoader from 'utils/image-res-loader';
import './style.scss';

/** Hoverable svg icons */
class HoverableIcon extends Component {
  constructor(props) {
    super(props);
  }

  render() {
    let continerClassName = "hoverable-icon-container " + this.props.className;
    let style = {};
    if (this.props.padding) {
      style.padding = this.props.padding;
    }
    if (this.props.isButton === "true"){
      style.cursor = 'pointer';
    }
    let notHoverImagePath = imageResLoader.getResPath(this.props.srcNotHover);
    let hoverImagePath = imageResLoader.getResPath(this.props.srcHover);

    return (
      <div className={continerClassName}>
        <img className="not-hover" src={notHoverImagePath} style={style} />
        <img className="hover" src={hoverImagePath} style={style} onClick={this.props.onClick} />
      </div>
    );
  }
}

export default HoverableIcon;
import React from 'react';
import deviceDetector from 'utils/device-detector';

function View(react, controller, componentModel){
    // PRIVATE

    // PUBLIC
    this.update = (newComponentModel) => {
        componentModel = newComponentModel;
        react.forceUpdate();
    }

    this.render = () => {
        return (
            <div id="top-nav-container">
            </div>
        );
    }
}

export default View;
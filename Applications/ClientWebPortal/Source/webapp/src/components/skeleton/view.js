import React from 'react';

function View(react, controller, componentModel){
    // PRIVATE

    // PUBLIC
    this.update = (newComponentModel) => {
        componentModel = newComponentModel;
        react.forceUpdate();
    }

    this.componentDidUpdate = () => {
        
    }

    this.render = () => {
        return (
            <div id="skeleton-container">
            </div>
        );
    }
}

export default View;
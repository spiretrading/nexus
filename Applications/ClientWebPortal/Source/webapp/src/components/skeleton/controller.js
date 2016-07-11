import View from './view';
import {browserHistory} from 'react-router/es6';
import userService from 'services/user';

function Controller(react){
    // PRIVATE
    var componentModel = {
    };

    var view = new View(react, this, clone(componentModel));
    
    // PUBLIC
    this.getView = () => {
        return view;
    }
}

export default Controller;
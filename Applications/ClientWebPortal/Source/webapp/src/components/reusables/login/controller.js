import View from './view';
import {browserHistory} from 'react-router/es6';
import userService from 'services/user';
import ResultCode from 'utils/spire-client/result-codes';

function Controller(react){
    // PRIVATE
    var componentModel = {
        isWaiting: false,
        isLoginSuccess: false
    };

    var view = new View(react, this, clone(componentModel));
    
    // PUBLIC
    this.getView = () => {
        return view;
    }

    this.login = (userId, password) => {
        let _this = this;
        componentModel.isWaiting = true;
        view.update(clone(componentModel));

        userService.login(userId, password)
            .then(onResult);

        function onResult(resultCode){
            if (resultCode === ResultCode.Success){
                browserHistory.push('/searchProfiles')
            }
            else{
                componentModel.isWaiting = false;
                componentModel.isLoginSuccess = false;
                view.update(clone(componentModel));
            }
        }
    }
}

export default Controller;
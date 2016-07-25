import React, {Component} from 'react';
import UserInfoNav from 'components/reusables/user-info-nav';
import routeParameters from 'utils/route-parameters';
import adminClient from 'utils/spire-clients/admin';
import preloaderTimer from 'utils/preloader-timer';
import moment from 'moment';
import PersonalDetails from 'components/reusables/personal-details';
import AccountPicture from 'components/reusables/account-picture';
import './style.scss';

class Account extends Component {
  constructor(props) {
    super(props);
    this.componentModel;
  }

  /** @private */
  getRequiredData() {
    let directoryEntry = routeParameters.get();
    let loadAccountRoles = adminClient.loadAccountRoles(directoryEntry);
    let loadAccountProfile = adminClient.loadAccountProfile(directoryEntry);

    return Promise.all([
      loadAccountRoles,
      loadAccountProfile
    ]);
  }

  componentDidMount() {
    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(requiredDataFetchPromise, null, 49, 60).then((responses) => {
      this.componentModel = responses[1];
      this.componentModel.roles = responses[0];
      this.forceUpdate();
    });
  }

  componentDidUpdate() {
    $('#account-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  render() {
    let content;
    let userInfoNavModel, lastSignin, personaldetailsModel, accountPictureModel;
    if (this.componentModel != null) {
      userInfoNavModel = {
        userName: this.componentModel.username,
        roles: this.componentModel.roles
      };

      personaldetailsModel = JSON.parse(JSON.stringify(this.componentModel));
      personaldetailsModel.showLabels = true;

      lastSignin = moment(this.componentModel.lastSignin, moment.ISO_8601).toDate().toLocaleString();

      accountPictureModel = {
        pictureUrl: this.componentModel.pictureUrl,
        showLabel: true
      };

      content =
        <div>
          <div className="row">
            <UserInfoNav model={userInfoNavModel}/>
          </div>
          <div className="row">
            Last sign-in: {lastSignin}
          </div>
          <div className="header row">
            Account Information
          </div>
          <div className="account-information-body row">
            <div className="account-picture-wrapper">
              <AccountPicture model={accountPictureModel} />
            </div>
            <div className="personal-details-wrapper">
              <PersonalDetails model={personaldetailsModel} />
            </div>
          </div>
          <div className="password-user-notes-wrapper">
            <div className="password-wrapper">
              <div className="header">
                Change Password
              </div>
            </div>
            <div className="user-notes-wrapper">
              <div className="header">
                User Notes
              </div>
              <div className="body">

              </div>
            </div>
          </div>
        </div>
    }

    return (
      <div id="account-container" className="container-fixed-width">
        {content}
      </div>
    );
  }
}

export default Account;

import './style.scss';
import React from 'react';
import ReactDOM from 'react-dom';
import UpdatableView from 'commons/updatable-view';
import deviceDetector from 'utils/device-detector';
import definitionsService from 'services/definitions';
import labelFormatter from 'utils/label-formatter';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  componentDidUpdate() {
    $('#group-account-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  onMemeberClicked(event) {
    let id = $(event.currentTarget).attr('data-id');
    this.controller.navigateToMemberProfile(id);
  }

  render() {
    let content;
    let members = [];
    if (this.controller.isModelInitialized()) {
      let numMembers = this.componentModel.members.length;
      let numTraders = 0;
      let numManagers = 0;
      let numAdmins = 0;
      for (let i=0; i<this.componentModel.members.length; i++) {
        let member = this.componentModel.members[i];
        if (member.roles.isTrader) {
          numTraders++;
        }
        if (member.roles.isManager) {
          numManagers++;
        }
        if (member.roles.isAdmin) {
          numAdmins++;
        }

        let defaultClass = 'icon icon-';
        let traderClass = defaultClass + 'trader';
        let managerClass = defaultClass + 'manager';
        let adminClass = defaultClass + 'admin';
        let serviceClass = defaultClass + 'server';

        if (member.roles.isTrader) {
          traderClass += ' active';
        }
        if (member.roles.isManager) {
          managerClass += ' active';
        }
        if (member.roles.isAdmin) {
          adminClass += ' active';
        }
        if (member.roles.isService) {
          serviceClass += ' active';
        }

        members.push(
          <div key={i} className="member-row" data-id={member.id} onClick={this.onMemeberClicked.bind(this)}>
            <div className="member-id">
              {member.name}
            </div>
            <div className="member-role">
              <span title="Trader" className={traderClass}></span>
              <span title="Manager" className={managerClass}></span>
              <span title="Admin" className={adminClass}></span>
              <span title="Service" className={serviceClass}></span>
            </div>
          </div>
        );
      }

      content =
        <div>
          <div className="user-info">
            {this.componentModel.groupName}
          </div>
          <div className="header">
            Group Information
          </div>
          <div className="body">
            <div className="traders">
              {members}
            </div>
            <div className="stats">
              <table>
                <tbody>
                  <tr>
                    <td className="stats-label">Total Members</td>
                    <td>{numMembers}</td>
                  </tr>
                  <tr>
                    <td className="stats-label">Total Traders</td>
                    <td>{numTraders}</td>
                  </tr>
                  <tr>
                    <td className="stats-label">Total Managers</td>
                    <td>{numManagers}</td>
                  </tr>
                  <tr>
                    <td className="stats-label">Total Admins</td>
                    <td>{numAdmins}</td>
                  </tr>
                </tbody>
              </table>
            </div>
          </div>
        </div>
    }

    let className = '';
    if (!deviceDetector.isMobile()) {
      className = 'container-fixed-width';
    }

    return (
      <div id="group-account-container" className={className}>
        {content}
      </div>
    );
  }
}

export default View;

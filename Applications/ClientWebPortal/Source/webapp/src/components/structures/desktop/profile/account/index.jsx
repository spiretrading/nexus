import {Component} from 'react';
import Controller from 'components/structures/common/profile/account/controller';
import View from './view';
import uuid from 'uuid';

class ProfileAccount extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    let componentModel = this.props.model || {};
    componentModel.componentId = uuid.v4();
    componentModel.directoryEntry = {
      id: parseInt(this.props.routeParams.id),
      name: this.props.routeParams.name,
      type: parseInt(this.props.routeParams.type)
    };
    this.controller = new Controller(componentModel);
    this.view = new View(this, this.controller, componentModel);
    this.controller.setView(this.view);
  }

  componentDidMount() {
    this.controller.componentDidMount();
  }

  componentDidUpdate() {
    let existingDirectoryEntry = this.controller.getDirectoryEntry();

    let id = parseInt(this.props.routeParams.id);
    let name = this.props.routeParams.name;
    let type = parseInt(this.props.routeParams.type);

    if (id != existingDirectoryEntry.id ||
      name != existingDirectoryEntry.name ||
      type != existingDirectoryEntry.type) {
      this.controller.setDirectoryEntry(type, id, name);
      this.controller.reloadAcountProfile();
    } else {
      this.controller.getView().componentDidUpdate();
    }
  }

  render() {
    return this.view.render();
  }
}

export default ProfileAccount;

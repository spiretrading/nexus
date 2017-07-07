import {AdministrationClient, ComplianceServiceClient, DirectoryEntry, DataType} from 'spire-client';
import preloaderTimer from 'utils/preloader-timer';
import userService from 'services/user';
import definitionsService from 'services/definitions';
import uuid from 'uuid';

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.componentModel.directoryEntry = new DirectoryEntry(
      this.componentModel.directoryEntry.id,
      this.componentModel.directoryEntry.type,
      this.componentModel.directoryEntry.name
    );
    this.adminClient = new AdministrationClient();
    this.complianceServiceClient = new ComplianceServiceClient();

    this.transformFromPerAccountRuleEntries = this.transformFromPerAccountRuleEntries.bind(this);
    this.initializeFlags = this.initializeFlags.bind(this);
    this.getSavePromises = this.getSavePromises.bind(this);
    this.onRuleAdd = this.onRuleAdd.bind(this);
    this.save = this.save.bind(this);
    this.isModelInitialized = this.isModelInitialized.bind(this);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  /** @private */
  getRequiredData() {
    let directoryEntry = this.componentModel.directoryEntry;
    let loadAccountRoles = this.adminClient.loadAccountRoles(directoryEntry);
    let loadComplianceSettings = this.complianceServiceClient.loadComplianceRuleEntries(directoryEntry);

    return Promise.all([
      loadComplianceSettings,
      loadAccountRoles
    ]);
  }

  /** @private */
  transformFromPerAccountRuleEntries(ruleEntries) {
    for (let i=0; i<ruleEntries.length; i++) {
      let ruleEntry = ruleEntries[i];
      // per account is a special case with multi-tier structure.
      // need to flatten the structure so the UI can understand
      // structure needs to be put back before saving to the backend
      if (ruleEntry.schema.name == 'per_account') {
        let ruleEntryName = getRuleEntryName(ruleEntry.schema.parameters);

        let transformedRuleEntry = {
          directory_entry: ruleEntry.directory_entry,
          id: ruleEntry.id,
          schema: {
            name: ruleEntryName,
            parameters: getRuleEntryParameters(ruleEntry.schema.parameters)
          },
          state: ruleEntry.state
        };

        ruleEntries[i] = transformedRuleEntry;
      }
    }
    return ruleEntries;

    function getRuleEntryName(parameters) {
      for (let i=0; i<parameters.length; i++) {
        if (parameters[i].name === 'name') {
          return parameters[i].value.value;
        }
      }
    }

    function getRuleEntryParameters(parameters) {
      let params = [];
      for (let i=0; i<parameters.length; i++) {
        if (parameters[i].name !== 'name') {
          params.push(parameters[i]);
        }
      }
      return params;
    }
  }

  /** @private */
  initializeFlags(ruleEntries) {
    for (let i=0; i<ruleEntries.length; i++) {
      let ruleEntry = ruleEntries[i];
      ruleEntry.toBeUpdated = false;
      ruleEntry.toBeAdded = false;
      ruleEntry.toBeDeleted = false;
    }

    return ruleEntries;
  }

  /** @private */
  transformToPerAccountRuleEntries(ruleEntries) {
    for (let i=0; i<ruleEntries.length; i++) {
      let ruleEntry = ruleEntries[i];
      if (this.componentModel.isGroup && (ruleEntry.state == 2 || ruleEntry.state == 3)) {
        let ruleEntryName = ruleEntry.schema.name;
        let parameters = shallowCopy(ruleEntry.schema.parameters);
        parameters.push({
          name: 'name',
          value: {
            value: ruleEntryName,
            which: 3
          }
        });

        let transformedRuleEntry = {
          directory_entry: ruleEntry.directory_entry,
          id: ruleEntry.id,
          schema: {
            name: 'per_account',
            parameters: parameters
          },
          state: ruleEntry.state
        };

        ruleEntries[i] = transformedRuleEntry;
      }
    }
    return ruleEntries;

    function shallowCopy(array) {
      let copy = [];
      for (let i=0; i<array.length; i++) {
        copy.push(array[i]);
      }
      return copy;
    }
  }

  componentDidMount() {
    let directoryEntry = this.componentModel.directoryEntry;
    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(requiredDataFetchPromise, null, Config.WHOLE_PAGE_PRELOADER_WIDTH, Config.WHOLE_PAGE_PRELOADER_HEIGHT).then((responses) => {
      let ruleEntries = responses[0];
      ruleEntries = this.transformFromPerAccountRuleEntries(ruleEntries);
      ruleEntries = this.initializeFlags(ruleEntries);
      this.componentModel.complianceRuleEntries = ruleEntries;
      this.componentModel.directoryEntry = directoryEntry;
      this.componentModel.roles = responses[1];
      this.componentModel.userName = directoryEntry.name;
      this.componentModel.isAdmin = userService.isAdmin();
      this.componentModel.isGroup = directoryEntry.type === 1;
      this.view.update(this.componentModel);
    });
  }

  isModelInitialized() {
    let model = clone(this.componentModel);
    delete model.componentId;
    delete model.directoryEntry;
    return !$.isEmptyObject(model);
  }

  onRuleUpdate(id, parameters, state) {
    let ruleEntries = this.componentModel.complianceRuleEntries;
    for (let i=0; i<ruleEntries.length; i++) {
      let entry = ruleEntries[i];
      if (entry.id == id) {
        entry.schema.parameters = parameters;
        entry.state = Number(state);

        if (state == 3) {
          entry.toBeDeleted = true;
          entry.toBeUpdated = false;
        } else {
          if (!entry.toBeAdded) {
            entry.toBeUpdated = true;
          }
          entry.toBeDeleted = false;
        }
      }
    }
  }

  onRuleAdd(ruleTypeName) {
    let schema = definitionsService.getComplianceRuleScehma(ruleTypeName);
    let id = uuid.v4();
    this.componentModel.complianceRuleEntries.push({
      id: id,
      schema: schema,
      state: 0,
      toBeAdded: true
    });
    this.view.update(this.componentModel);
  }

  save() {
    this.componentModel.complianceRuleEntries = this.transformToPerAccountRuleEntries(this.componentModel.complianceRuleEntries);
    let savePromises = this.getSavePromises();
    let allPromises = Promise.all(savePromises);

    preloaderTimer.start(allPromises, null, Config.WHOLE_PAGE_PRELOADER_WIDTH, Config.WHOLE_PAGE_PRELOADER_HEIGHT).then((responses) => {
      let ruleEntries = this.componentModel.complianceRuleEntries;
      let i = ruleEntries.length;
      while (i--) {
        if (ruleEntries[i].toBeDeleted) {
          ruleEntries.splice(i, 1);
        } else {
          ruleEntries[i].toBeUpdated = false;
          ruleEntries[i].toBeAdded = false;
          ruleEntries[i].toBeDeleted = false;
        }
      }
      this.view.update(this.componentModel);
      this.view.showSaveSuccessMessage();
    });
  }

  /** @private */
  getSavePromises() {
    let savePromises = [];
    for (let i=0; i<this.componentModel.complianceRuleEntries.length; i++) {
      let ruleEntry = this.componentModel.complianceRuleEntries[i];
      if (ruleEntry.toBeAdded && ruleEntry.toBeDeleted) {
        this.componentModel.complianceRuleEntries.splice(i, 1);
        i--;
      } else if (ruleEntry.toBeAdded) {
        let addPromise = this.complianceServiceClient.addComplianceRuleEntry(
          this.componentModel.directoryEntry,
          ruleEntry.state,
          clone(ruleEntry.schema));
        savePromises.push(addPromise);
      } else if (ruleEntry.toBeUpdated) {
        let apiRuleEntry = {
          directory_entry: this.componentModel.directoryEntry,
          id: ruleEntry.id,
          schema: clone(ruleEntry.schema),
          state: ruleEntry.state
        };
        let updatePromise = this.complianceServiceClient.updateComplianceRuleEntry(apiRuleEntry);
        savePromises.push(updatePromise);
      } else if (ruleEntry.toBeDeleted) {
        let deletePromise = this.complianceServiceClient.deleteComplianceRuleEntry(ruleEntry.id);
        savePromises.push(deletePromise);
      }
    }

    return savePromises;
  }
}

export default Controller;

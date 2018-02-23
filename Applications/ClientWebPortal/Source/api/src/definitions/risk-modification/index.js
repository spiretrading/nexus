import RiskParameters from '../../services/risk-service/risk-parameters';

class RiskModification {
    constructor(parameters) {
        this.parameters = parameters;
    }

    static fromData(data) {
        return new RiskModification(RiskParameters.fromData(data.parameters));
    }

    toData() {
        return {
            parameters: this.parameters.toData()
        };
    }
    
    clone() {
        return new RiskModification(this.parameters.clone());
    }
}

export default RiskModification;

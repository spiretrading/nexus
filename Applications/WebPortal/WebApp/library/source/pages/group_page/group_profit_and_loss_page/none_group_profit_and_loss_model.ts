import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { GroupProfitAndLossModel } from './group_profit_and_loss_model';

/** A GroupProfitAndLossModel that always returns an empty report. */
export class NoneGroupProfitAndLossModel extends GroupProfitAndLossModel {
  public async load(): Promise<void> {}

  public async startReport(
      _start: Beam.Date, _end: Beam.Date): Promise<number> {
    return 0;
  }

  public async awaitReport(
      _id: number): Promise<GroupProfitAndLossModel.Report> {
    return EMPTY_REPORT;
  }

  public async cancelReport(_id: number): Promise<void> {}
}

const EMPTY_REPORT: GroupProfitAndLossModel.Report = {
  totalProfitAndLoss: Nexus.Money.ZERO,
  totalFees: Nexus.Money.ZERO,
  totalVolume: Nexus.Quantity.ZERO,
  accounts: [],
  exchangeRates: []
};

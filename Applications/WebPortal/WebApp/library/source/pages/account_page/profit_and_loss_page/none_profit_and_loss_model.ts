import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { ProfitAndLossModel } from './profit_and_loss_model';

/** A ProfitAndLossModel that always returns an empty report. */
export class NoneProfitAndLossModel extends ProfitAndLossModel {

  public async load(): Promise<void> {}

  public async startReport(
      start: Beam.Date, end: Beam.Date): Promise<number> {
    return 0;
  }

  public async awaitReport(
      id: number): Promise<ProfitAndLossModel.Report> {
    return EMPTY_REPORT;
  }

  public async cancelReport(id: number): Promise<void> {}
}

const EMPTY_REPORT: ProfitAndLossModel.Report = {
  totalProfitAndLoss: Nexus.Money.ZERO,
  totalFees: Nexus.Money.ZERO,
  totalVolume: 0,
  currencies: [],
  exchangeRates: []
};

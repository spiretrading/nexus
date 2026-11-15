import * as Beam from 'beam';
import { InventoryUpdate } from './inventory_update';

/** A batch of updates to accounts' inventories. */
export class InventoryMessage extends Beam.Message {
  public static readonly TYPE = 'Nexus.RiskService.InventoryMessage';

  public static fromJson(value: any): InventoryMessage {
    return new InventoryMessage(
      Beam.arrayFromJson(InventoryUpdate, value.inventories));
  }

  /** The updated inventories. */
  public readonly inventories: InventoryUpdate[];

  /** Constructs a message containing a batch of inventory updates. */
  constructor(inventories: InventoryUpdate[]) {
    super(InventoryMessage.TYPE, {
      inventories: Beam.arrayToJson(inventories)
    });
    this.inventories = inventories;
  }
}

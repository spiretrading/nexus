import * as Beam from 'beam';
import { Inventory } from '../accounting/inventory';

/** Stores an update to an account's inventory. */
export class InventoryUpdate {

  /** Parses an InventoryUpdate from JSON. */
  public static fromJson(value: any): InventoryUpdate {
    return new InventoryUpdate(Beam.DirectoryEntry.fromJson(value.account),
      Inventory.fromJson(value.inventory));
  }

  /** The account whose inventory changed. */
  public account: Beam.DirectoryEntry;

  /** The updated inventory. */
  public inventory: Inventory;

  /** Constructs an update for an account's inventory. */
  constructor(account: Beam.DirectoryEntry, inventory: Inventory) {
    this.account = account;
    this.inventory = inventory;
  }

  /** Converts this update to JSON. */
  public toJson(): any {
    return {
      account: this.account.toJson(),
      inventory: this.inventory.toJson()
    };
  }
}

START TRANSACTION
  ALTER TABLE risk_parameters MODIFY COLUMN buying_power DOUBLE NOT NULL;
  ALTER TABLE risk_parameters MODIFY COLUMN net_loss DOUBLE NOT NULL;
COMMIT
START TRANSACTION
  ALTER TABLE risk_modifications MODIFY COLUMN buying_power DOUBLE NOT NULL;
  ALTER TABLE risk_modifications MODIFY COLUMN net_loss DOUBLE NOT NULL;
COMMIT
START TRANSACTION
  ALTER TABLE submissions MODIFY COLUMN quantity DOUBLE NOT NULL;
  UPDATE submissions SET quantity = 1000000 * quantity;
  ALTER TABLE submissions MODIFY COLUMN price DOUBLE NOT NULL;
COMMIT
START TRANSACTION
  ALTER TABLE execution_reports MODIFY COLUMN last_quantity DOUBLE NOT NULL;
  UPDATE execution_reports SET last_quantity = 1000000 * last_quantity;
  ALTER TABLE execution_reports MODIFY COLUMN last_price DOUBLE NOT NULL;
  ALTER TABLE execution_reports MODIFY COLUMN execution_fee DOUBLE NOT NULL;
  ALTER TABLE execution_reports MODIFY COLUMN processing_fee DOUBLE NOT NULL;
  ALTER TABLE execution_reports MODIFY COLUMN commission DOUBLE NOT NULL;
COMMIT

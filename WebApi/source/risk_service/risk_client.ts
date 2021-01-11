/** Client used to access risk services. */
export abstract class RiskClient {

  /** Connects to the service.
   * @throws ServiceError Indicates the connection failed.
   */
  public abstract open(): Promise<void>;

  /** Disconnects from the service. */
  public abstract close(): Promise<void>;
}

/** Resolves queries into values and suggestions. */
export abstract class QueryModel<T> {

  /** Resolves a query to one value.
   * @param query - The query to evaluate.
   * @return The resolved value, or null if no unique value can be resolved.
   */
  public abstract parse(query: string): Promise<T>;

  /** Finds possible matches for a query.
   * @param query - The query to evaluate. Its meaning is defined by the model.
   * @return The matching values in the model's preferred order.
   */
  public abstract submit(query: string): Promise<readonly T[]>;
}

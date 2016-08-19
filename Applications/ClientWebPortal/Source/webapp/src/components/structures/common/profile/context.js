/** Current profile page context for all the sub pages */
class Context {
  set(context) {
    this.values = context;
  }

  get() {
    return this.values;
  }
}

export default new Context();

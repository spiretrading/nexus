/** Bootstrap modal wrapper */
class Modal {
  constructor() {
    this.onModalClosed = null;
  }

  show($element, onModalOpened) {
    $element.modal({
      backdrop: 'static',
      keyboard: false
    }).on('shown.bs.modal', onShown.bind(this))
      .on('hidden.bs.modal', onHidden.bind(this))
      .show();

    function onShown(event) {
      $('.modal-backdrop').on('touchmove', (e) => {
        e.preventDefault();
      });

      if (onModalOpened != null) {
        onModalOpened();
      }
    }

    function onHidden(event) {
      $('.modal-backdrop').off();

      if (this.onModalClosed != null) {
        this.onModalClosed();
        this.onModalClosed = null;
      }
    }
  }

  hide($element, onModalClosed) {
    this.onModalClosed = onModalClosed;
    $element.modal('hide');
  }
}

export default new Modal();

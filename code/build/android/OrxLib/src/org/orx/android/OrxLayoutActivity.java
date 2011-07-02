package org.orx.android;

public abstract class OrxLayoutActivity extends OrxActivity {

	protected abstract int getLayoutID();
	protected abstract int getRenderSurfaceViewID();
	
	@Override
	protected void onSetContentView() {
		super.setContentView(this.getLayoutID());
		
		mView = (OrxView) findViewById(this.getRenderSurfaceViewID());
		/*
		 * Setup the context factory for 2.0 rendering. See ContextFactory class
		 * definition below
		 */
		mView.setEGLContextFactory(new OrxContextFactory());
		mView.setEGLWindowSurfaceFactory(new OrxWindowSurfaceFactory());

		mView.setRenderer(mRenderer);
		mView.setSynchroObject(mSynchroObject);
	}
}

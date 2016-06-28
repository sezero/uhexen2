extern void APIENTRY  __glutInit(__reg("a0") int *argcp,__reg("a1") char **argv,__reg("a6") void *)="\tjsr\t-36(a6)";
#define glutInit(x1,x2) __glutInit((x1),(x2),glutBase)
extern void APIENTRY  __glutInitDisplayMode(__reg("d0") unsigned int mode,__reg("a6") void *)="\tjsr\t-42(a6)";
#define glutInitDisplayMode(x1) __glutInitDisplayMode((x1),glutBase)
extern void APIENTRY  __glutInitDisplayString(__reg("a0") const char *string,__reg("a6") void *)="\tjsr\t-48(a6)";
#define glutInitDisplayString(x1) __glutInitDisplayString((x1),glutBase)
extern void APIENTRY  __glutInitWindowPosition(__reg("d0") int x,__reg("d1") int y,__reg("a6") void *)="\tjsr\t-54(a6)";
#define glutInitWindowPosition(x1,x2) __glutInitWindowPosition((x1),(x2),glutBase)
extern void APIENTRY  __glutInitWindowSize(__reg("d0") int width,__reg("d1") int height,__reg("a6") void *)="\tjsr\t-60(a6)";
#define glutInitWindowSize(x1,x2) __glutInitWindowSize((x1),(x2),glutBase)
extern void APIENTRY  __glutMainLoop(__reg("a6") void *)="\tjsr\t-66(a6)";
#define glutMainLoop() __glutMainLoop(glutBase)
extern int APIENTRY  __glutCreateWindow(__reg("a0") const char *title,__reg("a6") void *)="\tjsr\t-72(a6)";
#define glutCreateWindow(x1) __glutCreateWindow((x1),glutBase)
extern int APIENTRY  __glutCreateSubWindow(__reg("d0") int win,__reg("d1") int x,__reg("d2") int y,__reg("d3") int width,__reg("d4") int height,__reg("a6") void *)="\tjsr\t-78(a6)";
#define glutCreateSubWindow(x1,x2,x3,x4,x5) __glutCreateSubWindow((x1),(x2),(x3),(x4),(x5),glutBase)
extern void APIENTRY  __glutDestroyWindow(__reg("d0") int win,__reg("a6") void *)="\tjsr\t-84(a6)";
#define glutDestroyWindow(x1) __glutDestroyWindow((x1),glutBase)
extern void APIENTRY  __glutPostRedisplay(__reg("a6") void *)="\tjsr\t-90(a6)";
#define glutPostRedisplay() __glutPostRedisplay(glutBase)
extern void APIENTRY  __glutSwapBuffers(__reg("a6") void *)="\tjsr\t-96(a6)";
#define glutSwapBuffers() __glutSwapBuffers(glutBase)
extern int APIENTRY  __glutGetWindow(__reg("a6") void *)="\tjsr\t-102(a6)";
#define glutGetWindow() __glutGetWindow(glutBase)
extern void APIENTRY  __glutSetWindow(__reg("d0") int win,__reg("a6") void *)="\tjsr\t-108(a6)";
#define glutSetWindow(x1) __glutSetWindow((x1),glutBase)
extern void APIENTRY  __glutSetWindowTitle(__reg("a0") const char *title,__reg("a6") void *)="\tjsr\t-114(a6)";
#define glutSetWindowTitle(x1) __glutSetWindowTitle((x1),glutBase)
extern void APIENTRY  __glutSetIconTitle(__reg("a0") const char *title,__reg("a6") void *)="\tjsr\t-120(a6)";
#define glutSetIconTitle(x1) __glutSetIconTitle((x1),glutBase)
extern void APIENTRY  __glutPositionWindow(__reg("d0") int x,__reg("d1") int y,__reg("a6") void *)="\tjsr\t-126(a6)";
#define glutPositionWindow(x1,x2) __glutPositionWindow((x1),(x2),glutBase)
extern void APIENTRY  __glutReshapeWindow(__reg("d0") int width,__reg("d1") int height,__reg("a6") void *)="\tjsr\t-132(a6)";
#define glutReshapeWindow(x1,x2) __glutReshapeWindow((x1),(x2),glutBase)
extern void APIENTRY  __glutPopWindow(__reg("a6") void *)="\tjsr\t-138(a6)";
#define glutPopWindow() __glutPopWindow(glutBase)
extern void APIENTRY  __glutPushWindow(__reg("a6") void *)="\tjsr\t-144(a6)";
#define glutPushWindow() __glutPushWindow(glutBase)
extern void APIENTRY  __glutIconifyWindow(__reg("a6") void *)="\tjsr\t-150(a6)";
#define glutIconifyWindow() __glutIconifyWindow(glutBase)
extern void APIENTRY  __glutShowWindow(__reg("a6") void *)="\tjsr\t-156(a6)";
#define glutShowWindow() __glutShowWindow(glutBase)
extern void APIENTRY  __glutHideWindow(__reg("a6") void *)="\tjsr\t-162(a6)";
#define glutHideWindow() __glutHideWindow(glutBase)
extern void APIENTRY  __glutFullScreen(__reg("a6") void *)="\tjsr\t-168(a6)";
#define glutFullScreen() __glutFullScreen(glutBase)
extern void APIENTRY  __glutSetCursor(__reg("d0") int cursor,__reg("a6") void *)="\tjsr\t-174(a6)";
#define glutSetCursor(x1) __glutSetCursor((x1),glutBase)
extern void APIENTRY  __glutWarpPointer(__reg("d0") int x,__reg("d1") int y,__reg("a6") void *)="\tjsr\t-180(a6)";
#define glutWarpPointer(x1,x2) __glutWarpPointer((x1),(x2),glutBase)
extern void APIENTRY  __glutEstablishOverlay(__reg("a6") void *)="\tjsr\t-186(a6)";
#define glutEstablishOverlay() __glutEstablishOverlay(glutBase)
extern void APIENTRY  __glutRemoveOverlay(__reg("a6") void *)="\tjsr\t-192(a6)";
#define glutRemoveOverlay() __glutRemoveOverlay(glutBase)
extern void APIENTRY  __glutUseLayer(__reg("d0") GLenum layer,__reg("a6") void *)="\tjsr\t-198(a6)";
#define glutUseLayer(x1) __glutUseLayer((x1),glutBase)
extern void APIENTRY  __glutPostOverlayRedisplay(__reg("a6") void *)="\tjsr\t-204(a6)";
#define glutPostOverlayRedisplay() __glutPostOverlayRedisplay(glutBase)
extern void APIENTRY  __glutShowOverlay(__reg("a6") void *)="\tjsr\t-210(a6)";
#define glutShowOverlay() __glutShowOverlay(glutBase)
extern void APIENTRY  __glutHideOverlay(__reg("a6") void *)="\tjsr\t-216(a6)";
#define glutHideOverlay() __glutHideOverlay(glutBase)
extern int APIENTRY  __glutCreateMenu(__reg("a0") void (*)(int),__reg("a6") void *)="\tjsr\t-222(a6)";
#define glutCreateMenu(x1) __glutCreateMenu((x1),glutBase)
extern void APIENTRY  __glutDestroyMenu(__reg("d0") int menu,__reg("a6") void *)="\tjsr\t-228(a6)";
#define glutDestroyMenu(x1) __glutDestroyMenu((x1),glutBase)
extern int APIENTRY  __glutGetMenu(__reg("a6") void *)="\tjsr\t-234(a6)";
#define glutGetMenu() __glutGetMenu(glutBase)
extern void APIENTRY  __glutSetMenu(__reg("d0") int menu,__reg("a6") void *)="\tjsr\t-240(a6)";
#define glutSetMenu(x1) __glutSetMenu((x1),glutBase)
extern void APIENTRY  __glutAddMenuEntry(__reg("a0") const char *label,__reg("d0") int value,__reg("a6") void *)="\tjsr\t-246(a6)";
#define glutAddMenuEntry(x1,x2) __glutAddMenuEntry((x1),(x2),glutBase)
extern void APIENTRY  __glutAddSubMenu(__reg("a0") const char *label,__reg("d0") int submenu,__reg("a6") void *)="\tjsr\t-252(a6)";
#define glutAddSubMenu(x1,x2) __glutAddSubMenu((x1),(x2),glutBase)
extern void APIENTRY  __glutChangeToMenuEntry(__reg("d0") int item,__reg("a0") const char *label,__reg("d1") int value,__reg("a6") void *)="\tjsr\t-258(a6)";
#define glutChangeToMenuEntry(x1,x2,x3) __glutChangeToMenuEntry((x1),(x2),(x3),glutBase)
extern void APIENTRY  __glutChangeToSubMenu(__reg("d0") int item,__reg("a0") const char *label,__reg("d1") int submenu,__reg("a6") void *)="\tjsr\t-264(a6)";
#define glutChangeToSubMenu(x1,x2,x3) __glutChangeToSubMenu((x1),(x2),(x3),glutBase)
extern void APIENTRY  __glutRemoveMenuItem(__reg("d0") int item,__reg("a6") void *)="\tjsr\t-270(a6)";
#define glutRemoveMenuItem(x1) __glutRemoveMenuItem((x1),glutBase)
extern void APIENTRY  __glutAttachMenu(__reg("d0") int button,__reg("a6") void *)="\tjsr\t-276(a6)";
#define glutAttachMenu(x1) __glutAttachMenu((x1),glutBase)
extern void APIENTRY  __glutDetachMenu(__reg("d0") int button,__reg("a6") void *)="\tjsr\t-282(a6)";
#define glutDetachMenu(x1) __glutDetachMenu((x1),glutBase)
extern void APIENTRY  __glutDisplayFunc(__reg("a0") void (*func)(void),__reg("a6") void *)="\tjsr\t-288(a6)";
#define glutDisplayFunc(x1) __glutDisplayFunc((x1),glutBase)
extern void APIENTRY  __glutReshapeFunc(__reg("a0") void (*func)(int width, int height),__reg("a6") void *)="\tjsr\t-294(a6)";
#define glutReshapeFunc(x1) __glutReshapeFunc((x1),glutBase)
extern void APIENTRY  __glutKeyboardFunc(__reg("a0") void (*func)(unsigned char key, int x, int y),__reg("a6") void *)="\tjsr\t-300(a6)";
#define glutKeyboardFunc(x1) __glutKeyboardFunc((x1),glutBase)
extern void APIENTRY  __glutMouseFunc(__reg("a0") void (*func)(int button, int state, int x, int y),__reg("a6") void *)="\tjsr\t-306(a6)";
#define glutMouseFunc(x1) __glutMouseFunc((x1),glutBase)
extern void APIENTRY  __glutMotionFunc(__reg("a0") void (*func)(int x, int y),__reg("a6") void *)="\tjsr\t-312(a6)";
#define glutMotionFunc(x1) __glutMotionFunc((x1),glutBase)
extern void APIENTRY  __glutPassiveMotionFunc(__reg("a0") void (*func)(int x, int y),__reg("a6") void *)="\tjsr\t-318(a6)";
#define glutPassiveMotionFunc(x1) __glutPassiveMotionFunc((x1),glutBase)
extern void APIENTRY  __glutEntryFunc(__reg("a0") void (*func)(int state),__reg("a6") void *)="\tjsr\t-324(a6)";
#define glutEntryFunc(x1) __glutEntryFunc((x1),glutBase)
extern void APIENTRY  __glutVisibilityFunc(__reg("a0") void (*func)(int state),__reg("a6") void *)="\tjsr\t-330(a6)";
#define glutVisibilityFunc(x1) __glutVisibilityFunc((x1),glutBase)
extern void APIENTRY  __glutIdleFunc(__reg("a0") void (*func)(void),__reg("a6") void *)="\tjsr\t-336(a6)";
#define glutIdleFunc(x1) __glutIdleFunc((x1),glutBase)
extern void APIENTRY  __glutTimerFunc(__reg("d0") unsigned int millis,__reg("a0") void (*func)(int value),__reg("d1") int value,__reg("a6") void *)="\tjsr\t-342(a6)";
#define glutTimerFunc(x1,x2,x3) __glutTimerFunc((x1),(x2),(x3),glutBase)
extern void APIENTRY  __glutMenuStateFunc(__reg("a0") void (*func)(int state),__reg("a6") void *)="\tjsr\t-348(a6)";
#define glutMenuStateFunc(x1) __glutMenuStateFunc((x1),glutBase)
extern void APIENTRY  __glutSpecialFunc(__reg("a0") void (*func)(int key, int x, int y),__reg("a6") void *)="\tjsr\t-354(a6)";
#define glutSpecialFunc(x1) __glutSpecialFunc((x1),glutBase)
extern void APIENTRY  __glutSpaceballMotionFunc(__reg("a0") void (*func)(int x, int y, int z),__reg("a6") void *)="\tjsr\t-360(a6)";
#define glutSpaceballMotionFunc(x1) __glutSpaceballMotionFunc((x1),glutBase)
extern void APIENTRY  __glutSpaceballRotateFunc(__reg("a0") void (*func)(int x, int y, int z),__reg("a6") void *)="\tjsr\t-366(a6)";
#define glutSpaceballRotateFunc(x1) __glutSpaceballRotateFunc((x1),glutBase)
extern void APIENTRY  __glutSpaceballButtonFunc(__reg("a0") void (*func)(int button, int state),__reg("a6") void *)="\tjsr\t-372(a6)";
#define glutSpaceballButtonFunc(x1) __glutSpaceballButtonFunc((x1),glutBase)
extern void APIENTRY  __glutButtonBoxFunc(__reg("a0") void (*func)(int button, int state),__reg("a6") void *)="\tjsr\t-378(a6)";
#define glutButtonBoxFunc(x1) __glutButtonBoxFunc((x1),glutBase)
extern void APIENTRY  __glutDialsFunc(__reg("a0") void (*func)(int dial, int value),__reg("a6") void *)="\tjsr\t-384(a6)";
#define glutDialsFunc(x1) __glutDialsFunc((x1),glutBase)
extern void APIENTRY  __glutTabletMotionFunc(__reg("a0") void (*func)(int x, int y),__reg("a6") void *)="\tjsr\t-390(a6)";
#define glutTabletMotionFunc(x1) __glutTabletMotionFunc((x1),glutBase)
extern void APIENTRY  __glutTabletButtonFunc(__reg("a0") void (*func)(int button, int state, int x, int y),__reg("a6") void *)="\tjsr\t-396(a6)";
#define glutTabletButtonFunc(x1) __glutTabletButtonFunc((x1),glutBase)
extern void APIENTRY  __glutMenuStatusFunc(__reg("a0") void (*func)(int status, int x, int y),__reg("a6") void *)="\tjsr\t-402(a6)";
#define glutMenuStatusFunc(x1) __glutMenuStatusFunc((x1),glutBase)
extern void APIENTRY  __glutOverlayDisplayFunc(__reg("a0") void (*func)(void),__reg("a6") void *)="\tjsr\t-408(a6)";
#define glutOverlayDisplayFunc(x1) __glutOverlayDisplayFunc((x1),glutBase)
extern void APIENTRY  __glutWindowStatusFunc(__reg("a0") void (*func)(int state),__reg("a6") void *)="\tjsr\t-414(a6)";
#define glutWindowStatusFunc(x1) __glutWindowStatusFunc((x1),glutBase)
extern void APIENTRY  __glutSetColor(__reg("d0") int,__reg("fp0") GLfloat red,__reg("fp1") GLfloat green,__reg("fp2") GLfloat blue,__reg("a6") void *)="\tjsr\t-420(a6)";
#define glutSetColor(x1,x2,x3,x4) __glutSetColor((x1),(x2),(x3),(x4),glutBase)
extern GLfloat APIENTRY  __glutGetColor(__reg("d0") int ndx,__reg("d1") int component,__reg("a6") void *)="\tjsr\t-426(a6)";
#define glutGetColor(x1,x2) __glutGetColor((x1),(x2),glutBase)
extern void APIENTRY  __glutCopyColormap(__reg("d0") int win,__reg("a6") void *)="\tjsr\t-432(a6)";
#define glutCopyColormap(x1) __glutCopyColormap((x1),glutBase)
extern int APIENTRY  __glutGet(__reg("d0") GLenum type,__reg("a6") void *)="\tjsr\t-438(a6)";
#define glutGet(x1) __glutGet((x1),glutBase)
extern int APIENTRY  __glutDeviceGet(__reg("d0") GLenum type,__reg("a6") void *)="\tjsr\t-444(a6)";
#define glutDeviceGet(x1) __glutDeviceGet((x1),glutBase)
extern int APIENTRY  __glutExtensionSupported(__reg("a0") const char *name,__reg("a6") void *)="\tjsr\t-450(a6)";
#define glutExtensionSupported(x1) __glutExtensionSupported((x1),glutBase)
extern int APIENTRY  __glutGetModifiers(__reg("a6") void *)="\tjsr\t-456(a6)";
#define glutGetModifiers() __glutGetModifiers(glutBase)
extern int APIENTRY  __glutLayerGet(__reg("d0") GLenum type,__reg("a6") void *)="\tjsr\t-462(a6)";
#define glutLayerGet(x1) __glutLayerGet((x1),glutBase)
extern void APIENTRY  __glutBitmapCharacter(__reg("a0") void *font,__reg("d0") int character,__reg("a6") void *)="\tjsr\t-468(a6)";
#define glutBitmapCharacter(x1,x2) __glutBitmapCharacter((x1),(x2),glutBase)
extern int APIENTRY  __glutBitmapWidth(__reg("a0") void *font,__reg("d0") int character,__reg("a6") void *)="\tjsr\t-474(a6)";
#define glutBitmapWidth(x1,x2) __glutBitmapWidth((x1),(x2),glutBase)
extern void APIENTRY  __glutStrokeCharacter(__reg("a0") void *font,__reg("d0") int character,__reg("a6") void *)="\tjsr\t-480(a6)";
#define glutStrokeCharacter(x1,x2) __glutStrokeCharacter((x1),(x2),glutBase)
extern int APIENTRY  __glutStrokeWidth(__reg("a0") void *font,__reg("d0") int character,__reg("a6") void *)="\tjsr\t-486(a6)";
#define glutStrokeWidth(x1,x2) __glutStrokeWidth((x1),(x2),glutBase)
extern int APIENTRY  __glutBitmapLength(__reg("a0") void *font,__reg("a1") const unsigned char *string,__reg("a6") void *)="\tjsr\t-492(a6)";
#define glutBitmapLength(x1,x2) __glutBitmapLength((x1),(x2),glutBase)
extern int APIENTRY  __glutStrokeLength(__reg("a0") void *font,__reg("a1") const unsigned char *string,__reg("a6") void *)="\tjsr\t-498(a6)";
#define glutStrokeLength(x1,x2) __glutStrokeLength((x1),(x2),glutBase)
extern void APIENTRY  __glutWireSphere(__reg("fp0") GLdouble radius,__reg("d0") GLint slices,__reg("d1") GLint stacks,__reg("a6") void *)="\tjsr\t-504(a6)";
#define glutWireSphere(x1,x2,x3) __glutWireSphere((x1),(x2),(x3),glutBase)
extern void APIENTRY  __glutSolidSphere(__reg("fp0") GLdouble radius,__reg("d0") GLint slices,__reg("d1") GLint stacks,__reg("a6") void *)="\tjsr\t-510(a6)";
#define glutSolidSphere(x1,x2,x3) __glutSolidSphere((x1),(x2),(x3),glutBase)
extern void APIENTRY  __glutWireCone(__reg("fp0") GLdouble base,__reg("fp1") GLdouble height,__reg("d0") GLint slices,__reg("d1") GLint stacks,__reg("a6") void *)="\tjsr\t-516(a6)";
#define glutWireCone(x1,x2,x3,x4) __glutWireCone((x1),(x2),(x3),(x4),glutBase)
extern void APIENTRY  __glutSolidCone(__reg("fp0") GLdouble base,__reg("fp1") GLdouble height,__reg("d0") GLint slices,__reg("d1") GLint stacks,__reg("a6") void *)="\tjsr\t-522(a6)";
#define glutSolidCone(x1,x2,x3,x4) __glutSolidCone((x1),(x2),(x3),(x4),glutBase)
extern void APIENTRY  __glutWireCube(__reg("fp0") GLdouble size,__reg("a6") void *)="\tjsr\t-528(a6)";
#define glutWireCube(x1) __glutWireCube((x1),glutBase)
extern void APIENTRY  __glutSolidCube(__reg("fp0") GLdouble size,__reg("a6") void *)="\tjsr\t-534(a6)";
#define glutSolidCube(x1) __glutSolidCube((x1),glutBase)
extern void APIENTRY  __glutWireTorus(__reg("fp0") GLdouble innerRadius,__reg("fp1") GLdouble outerRadius,__reg("d0") GLint sides,__reg("d1") GLint rings,__reg("a6") void *)="\tjsr\t-540(a6)";
#define glutWireTorus(x1,x2,x3,x4) __glutWireTorus((x1),(x2),(x3),(x4),glutBase)
extern void APIENTRY  __glutSolidTorus(__reg("fp0") GLdouble innerRadius,__reg("fp1") GLdouble outerRadius,__reg("d0") GLint sides,__reg("d1") GLint rings,__reg("a6") void *)="\tjsr\t-546(a6)";
#define glutSolidTorus(x1,x2,x3,x4) __glutSolidTorus((x1),(x2),(x3),(x4),glutBase)
extern void APIENTRY  __glutWireDodecahedron(__reg("a6") void *)="\tjsr\t-552(a6)";
#define glutWireDodecahedron() __glutWireDodecahedron(glutBase)
extern void APIENTRY  __glutSolidDodecahedron(__reg("a6") void *)="\tjsr\t-558(a6)";
#define glutSolidDodecahedron() __glutSolidDodecahedron(glutBase)
extern void APIENTRY  __glutWireTeapot(__reg("fp0") GLdouble size,__reg("a6") void *)="\tjsr\t-564(a6)";
#define glutWireTeapot(x1) __glutWireTeapot((x1),glutBase)
extern void APIENTRY  __glutSolidTeapot(__reg("fp0") GLdouble size,__reg("a6") void *)="\tjsr\t-570(a6)";
#define glutSolidTeapot(x1) __glutSolidTeapot((x1),glutBase)
extern void APIENTRY  __glutWireOctahedron(__reg("a6") void *)="\tjsr\t-576(a6)";
#define glutWireOctahedron() __glutWireOctahedron(glutBase)
extern void APIENTRY  __glutSolidOctahedron(__reg("a6") void *)="\tjsr\t-582(a6)";
#define glutSolidOctahedron() __glutSolidOctahedron(glutBase)
extern void APIENTRY  __glutWireTetrahedron(__reg("a6") void *)="\tjsr\t-588(a6)";
#define glutWireTetrahedron() __glutWireTetrahedron(glutBase)
extern void APIENTRY  __glutSolidTetrahedron(__reg("a6") void *)="\tjsr\t-594(a6)";
#define glutSolidTetrahedron() __glutSolidTetrahedron(glutBase)
extern void APIENTRY  __glutWireIcosahedron(__reg("a6") void *)="\tjsr\t-600(a6)";
#define glutWireIcosahedron() __glutWireIcosahedron(glutBase)
extern void APIENTRY  __glutSolidIcosahedron(__reg("a6") void *)="\tjsr\t-606(a6)";
#define glutSolidIcosahedron() __glutSolidIcosahedron(glutBase)
extern int APIENTRY  __glutVideoResizeGet(__reg("d0") GLenum param,__reg("a6") void *)="\tjsr\t-612(a6)";
#define glutVideoResizeGet(x1) __glutVideoResizeGet((x1),glutBase)
extern void APIENTRY  __glutSetupVideoResizing(__reg("a6") void *)="\tjsr\t-618(a6)";
#define glutSetupVideoResizing() __glutSetupVideoResizing(glutBase)
extern void APIENTRY  __glutStopVideoResizing(__reg("a6") void *)="\tjsr\t-624(a6)";
#define glutStopVideoResizing() __glutStopVideoResizing(glutBase)
extern void APIENTRY  __glutVideoResize(__reg("d0") int x,__reg("d1") int y,__reg("d2") int width,__reg("d3") int height,__reg("a6") void *)="\tjsr\t-630(a6)";
#define glutVideoResize(x1,x2,x3,x4) __glutVideoResize((x1),(x2),(x3),(x4),glutBase)
extern void APIENTRY  __glutVideoPan(__reg("d0") int x,__reg("d1") int y,__reg("d2") int width,__reg("d3") int height,__reg("a6") void *)="\tjsr\t-636(a6)";
#define glutVideoPan(x1,x2,x3,x4) __glutVideoPan((x1),(x2),(x3),(x4),glutBase)
extern void APIENTRY  __glutReportErrors(__reg("a6") void *)="\tjsr\t-642(a6)";
#define glutReportErrors() __glutReportErrors(glutBase)
extern void APIENTRY  __glutKeyboardUpFunc(__reg("a0") void (*func)(unsigned char key, int x, int y),__reg("a6") void *)="\tjsr\t-648(a6)";
#define glutKeyboardUpFunc(x1) __glutKeyboardUpFunc((x1),glutBase)
extern void APIENTRY  __glutSpecialUpFunc(__reg("a0") void (*func)(int key, int x, int y),__reg("a6") void *)="\tjsr\t-654(a6)";
#define glutSpecialUpFunc(x1) __glutSpecialUpFunc((x1),glutBase)
extern void APIENTRY  __glutJoystickFunc(__reg("a0") void (*func)(unsigned int buttonMask, int x, int y, int z),__reg("d0") int pollInterval,__reg("a6") void *)="\tjsr\t-660(a6)";
#define glutJoystickFunc(x1,x2) __glutJoystickFunc((x1),(x2),glutBase)
extern void APIENTRY  __glutIgnoreKeyRepeat(__reg("d0") int ignore,__reg("a6") void *)="\tjsr\t-666(a6)";
#define glutIgnoreKeyRepeat(x1) __glutIgnoreKeyRepeat((x1),glutBase)
extern void APIENTRY  __glutSetKeyRepeat(__reg("d0") int repeatMode,__reg("a6") void *)="\tjsr\t-672(a6)";
#define glutSetKeyRepeat(x1) __glutSetKeyRepeat((x1),glutBase)
extern void APIENTRY  __glutForceJoystickFunc(__reg("a6") void *)="\tjsr\t-678(a6)";
#define glutForceJoystickFunc() __glutForceJoystickFunc(glutBase)
extern int APIENTRY  __glutEnterGameMode(__reg("a6") void *)="\tjsr\t-684(a6)";
#define glutEnterGameMode() __glutEnterGameMode(glutBase)
extern void APIENTRY  __glutLeaveGameMode(__reg("a6") void *)="\tjsr\t-690(a6)";
#define glutLeaveGameMode() __glutLeaveGameMode(glutBase)
extern int APIENTRY  __glutGameModeGet(__reg("d0") GLenum mode,__reg("a6") void *)="\tjsr\t-696(a6)";
#define glutGameModeGet(x1) __glutGameModeGet((x1),glutBase)
extern void APIENTRY  __glutGameModeString(__reg("a0") const char *string,__reg("a6") void *)="\tjsr\t-702(a6)";
#define glutGameModeString(x1) __glutGameModeString((x1),glutBase)
extern void APIENTRY  __glutPostWindowRedisplay(__reg("d0") int win,__reg("a6") void *)="\tjsr\t-708(a6)";
#define glutPostWindowRedisplay(x1) __glutPostWindowRedisplay((x1),glutBase)
extern void APIENTRY  __glutPostWindowOverlayRedisplay(__reg("d0") int win,__reg("a6") void *)="\tjsr\t-714(a6)";
#define glutPostWindowOverlayRedisplay(x1) __glutPostWindowOverlayRedisplay((x1),glutBase)

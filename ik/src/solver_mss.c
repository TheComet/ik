#include "ik/effector.h"
#include "ik/solver.h"
#include "ik/log.h"

struct ik_solver_mss
{
    IK_SOLVER_HEAD
};

/* ------------------------------------------------------------------------- */
static int
mss_init(struct ik_solver* solver_base, const struct ik_subtree* subtree)
{
    ik_log_printf(IK_DEBUG, "MSS: Initialized");

    return 0;
}

/* ------------------------------------------------------------------------- */
static void
mss_deinit(struct ik_solver* solver_base)
{

}

/* ------------------------------------------------------------------------- */
static int
mss_solve(struct ik_solver* solver_base)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
static void
mss_iterate_nodes(const struct ik_solver* solver_base, ik_solver_callback_func callback, int skip_base)
{

}

/* ------------------------------------------------------------------------- */
static void
mss_iterate_effector_nodes(const struct ik_solver* solver_base, ik_solver_callback_func callback)
{

}

/* ------------------------------------------------------------------------- */
static void
mss_get_first_segment(const struct ik_solver* solver_base, struct ik_node** base, struct ik_node** tip)
{
    struct ik_solver_mss* solver = (struct ik_solver_mss*)solver_base;
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_MSS = {
    "mss",
    sizeof(struct ik_solver_mss),
    mss_init,
    mss_deinit,
    mss_solve,
    mss_iterate_nodes,
    mss_iterate_effector_nodes,
    mss_get_first_segment
};

#if 0

#define SAFETY 0.9
#define PGROW -0.2
#define PSHRNK -0.25
#define ERRCON 1.89e-4

#define a2  (0.2)
#define a3  (0.3)
#define a4  (0.6)
#define a5  (1.0)
#define a6  (0.875)
#define b21 (0.2)
#define b31 (3.0 / 40.0)
#define b32 (9.0 / 40.0)
#define b41 (0.3)
#define b42 (-0.9)
#define b43 (1.2)
#define b51 (-11.0 / 54.0)
#define b52 (2.5)
#define b53 (-70.0 / 27.0)
#define b54 (35.0 / 27.0)
#define b61 (1631.0 / 55296.0)
#define b62 (175.0 / 512.0)
#define b63 (575.0 / 13824.0)
#define b64 (44275.0 / 110592.0)
#define b65 (253.0 / 4096.0)
#define c1  (37.0 / 378.0)
#define c3  (250.0 / 621.0)
#define c4  (125.0 / 594.0)
#define c6  (512.0 / 1771.0)
#define dc1 (c1 - 2825.0 / 27648.0)
#define dc3 (c3 - 18575.0 / 48384.0)
#define dc4 (c4 - 13525.0 / 55296.0)
#define dc5 (-277.00 / 14336.0)
#define dc6 (c6 - 0.25)

/* The value ERRCON equals (5/SAFETY) raised to the power (1/PGROW), see use below. */

/* Given values for n variables y[1..n] and their derivatives dydx[1..n] known at x, use
the fifth-order Cash-Karp Runge-Kutta method to advance the solution over an interval h
and return the incremented variables as yout[1..n]. Also return an estimate of the local
truncation error in yout using the embedded fourth-order method. The user supplies the routine
derivs(x,y,dydx), which returns derivatives dydx at x. */

void rkck(float y[], float dydx[], int n, float x, float h, float yout[],
          float yerr[], void (*derivs)(float, float [], float []))

{
    int i;

    float *ak2,*ak3,*ak4,*ak5,*ak6,*ytemp; /*
    ak2=vector(1,n);
    ak3=vector(1,n);
    ak4=vector(1,n);
    ak5=vector(1,n);
    ak6=vector(1,n);
    ytemp=vector(1,n);*/

    for (i=1;i<=n;i++) /*First step.*/
        ytemp[i]=y[i]+b21*h*dydx[i];
    (*derivs)(x+a2*h,ytemp,ak2); /*Second step.*/
    for (i=1;i<=n;i++)
        ytemp[i]=y[i]+h*(b31*dydx[i]+b32*ak2[i]);
    (*derivs)(x+a3*h,ytemp,ak3); /*Third step.*/
    for (i=1;i<=n;i++)
        ytemp[i]=y[i]+h*(b41*dydx[i]+b42*ak2[i]+b43*ak3[i]);
    (*derivs)(x+a4*h,ytemp,ak4); /*Fourth step.*/
    for (i=1;i<=n;i++)
        ytemp[i]=y[i]+h*(b51*dydx[i]+b52*ak2[i]+b53*ak3[i]+b54*ak4[i]);
    (*derivs)(x+a5*h,ytemp,ak5); /*Fifth step.*/
    for (i=1;i<=n;i++)
        ytemp[i]=y[i]+h*(b61*dydx[i]+b62*ak2[i]+b63*ak3[i]+b64*ak4[i]+b65*ak5[i]);
    (*derivs)(x+a6*h,ytemp,ak6); /*Sixth step.*/
    for (i=1;i<=n;i++) /*Accumulate increments with proper weights.*/
        yout[i]=y[i]+h*(c1*dydx[i]+c3*ak3[i]+c4*ak4[i]+c6*ak6[i]);
    /* Estimate error as difference between fourth and fifth order methods. */
    for (i=1;i<=n;i++)
        yerr[i]=h*(dc1*dydx[i]+dc3*ak3[i]+dc4*ak4[i]+dc5*ak5[i]+dc6*ak6[i]);
    /*
    free_vector(ytemp,1,n);
    free_vector(ak6,1,n);
    free_vector(ak5,1,n);
    free_vector(ak4,1,n);
    free_vector(ak3,1,n);
    free_vector(ak2,1,n);*/
}

/*
 Fifth-order Runge-Kutta step with monitoring of local truncation error to ensure accuracy and
 adjust stepsize. Input are the dependent variable vector y[1..n] and its derivative dydx[1..n]
 at the starting value of the independent variable x. Also input are the stepsize to be attempted
 htry, the required accuracy eps, and the vector yscal[1..n] against which the error is
 scaled. On output, y and x are replaced by their new values, hdid is the stepsize that was
 actually accomplished, and hnext is the estimated next stepsize. derivs is the user-supplied
 routine that computes the right-hand side derivatives.*/

void rkqs(float y[], float dydx[], int n, float *x, float htry, float eps,
          float yscal[], float *hdid, float *hnext,
          void (*derivs)(float, float [], float []))
{
    int i;
    float errmax,h,htemp,xnew,*yerr,*ytemp;
    /*
    yerr=vector(1,n);
    ytemp=vector(1,n);*/

    h = htry; /* Set stepsize to the initial trial value. */
    for (;;)
    {
        rkck(y,dydx,n,*x,h,ytemp,yerr,derivs); /* Take a step. */
        errmax = 0.0; /* Evaluate accuracy. */
        for (i = 1; i <= n; i++)
            errmax = fmax(errmax, fabs(yerr[i]/yscal[i]));
        errmax /= eps; /* Scale relative to required tolerance. */
        if (errmax <= 1.0)
            break; /* Step succeeded. Compute size of next step. */
        htemp=SAFETY*h*pow(errmax, PSHRNK);
        /* Truncation error too large, reduce stepsize. */
        h = (h >= 0.0 ? fmax(htemp, 0.1*h) : fmin(htemp, 0.1*h));
        /* No more than a factor of 10. */
        xnew = (*x)+h;
        if (xnew == *x)
        {
            ik.log.message("stepsize underflow in rkqs");
            return;
        }
    }

    if (errmax > ERRCON)
        *hnext=SAFETY*h*pow(errmax,PGROW);
    else
        *hnext=5.0*h; /* No more than a factor of 5 increase. */
    *x += (*hdid=h);
    for (i=1;i<=n;i++)
        y[i]=ytemp[i];
    /*
    free_vector(ytemp,1,n);
    free_vector(yerr,1,n);*/
}

#define MAXSTP 10000
#define TINY 1.0e-30
#define SIGN(x) (((x) > 0) - ((x) < 0))


/* User storage for intermediate results. Preset kmax and dxsav in the calling program. If kmax 6=
0 results are stored at approximate intervals dxsav in the arrays xp[1..kount], yp[1..nvar]
[1..kount], where kount is output by odeint. Defining declarations for these variables, with
memory allocations xp[1..kmax] and yp[1..nvar][1..kmax] for the arrays, should be in
the calling program. */

extern int kmax,kount;
extern float *xp,**yp,dxsav;

/* Runge-Kutta driver with adaptive stepsize control. Integrate starting values ystart[1..nvar]
from x1 to x2 with accuracy eps, storing intermediate results in global variables. h1 should
be set as a guessed first stepsize, hmin as the minimum allowed stepsize (can be zero). On
output nok and nbad are the number of good and bad (but retried and fixed) steps taken, and
ystart is replaced by values at the end of the integration interval. derivs is the user-supplied
routine for calculating the right-hand side derivative, while rkqs is the name of the stepper
routine to be used. */

void odeint(float ystart[], int nvar, float x1, float x2, float eps, float h1,
            float hmin, int *nok, int *nbad,
            void (*derivs)(float, float [], float []))
{
    int nstp,i;
    float xsav,x,hnext,hdid,h;
    float *yscal,*y,*dydx;
    /*
    yscal=vector(1,nvar);
    y=vector(1,nvar);
    dydx=vector(1,nvar);*/
    x=x1;
    /*h=SIGN(h1,x2-x1);*/
    h=h1;
    *nok = (*nbad) = kount = 0;
    for (i=1;i<=nvar;i++)
        y[i]=ystart[i];
    if (kmax > 0) xsav=x-dxsav*2.0;   /*Assures storage of first step.*/
    for (nstp=1;nstp<=MAXSTP;nstp++)  /*Take at most MAXSTP steps.*/
    {
        (*derivs)(x,y,dydx);
        /*Scaling used to monitor accuracy. This general-purpose choice can be modified
        if need be.*/
        for (i=1;i<=nvar;i++)
            yscal[i]=fabs(y[i])+fabs(dydx[i]*h)+TINY;
        if (kmax > 0 && kount < kmax-1 && fabs(x-xsav) > fabs(dxsav))
        {
            xp[++kount]=x; /*Store intermediate results.*/
            for (i=1;i<=nvar;i++)
                yp[i][kount]=y[i];
            xsav=x;
        }
        if ((x+h-x2)*(x+h-x1) > 0.0)
            h=x2-x; /*If stepsize can overshoot, decrease*/
        rkqs(y,dydx,nvar,&x,h,eps,yscal,&hdid,&hnext,derivs);
        if (hdid == h)
            ++(*nok);
        else
            ++(*nbad);
        if ((x-x2)*(x2-x1) >= 0.0) /*Are we done?*/
        {
            for (i=1;i<=nvar;i++)
                ystart[i]=y[i];
            if (kmax)
            {
                xp[++kount]=x; /*Save final step.*/
                for (i=1;i<=nvar;i++)
                    yp[i][kount]=y[i];
            }
            /*
            free_vector(dydx,1,nvar);
            free_vector(y,1,nvar);
            free_vector(yscal,1,nvar);*/
            return; /*Normal exit.*/
        }
        if (fabs(hnext) <= hmin)
        {
            ik.log.message("Step size too small in odeint");
            return;
        }
        h=hnext;
    }

    ik.log.message("Too many steps in routine odeint");
}
#endif

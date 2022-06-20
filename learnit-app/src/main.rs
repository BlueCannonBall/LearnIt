use yew::prelude::*;
use yew_router::prelude::*;

mod navbar;
use navbar::*;

#[function_component(Home)]
fn home() -> Html {
    let history = use_history().unwrap();

    let onclick = Callback::once(move |r: Route| history.push(Route::Home));
    html! {
        <>
            <NavBar />

            <div class="jumbotron">
                <h1 style="margin-top:70px; text-align: center;">
                    { "LearnIt! uses proven science to accelerate your learning." }
                </h1>

                <center>
                    <p>
                        { "See the results for yourself. " } <Link<Route> to={Route::SignUp}><linkeffect>{ "Sign up" }</linkeffect></Link<Route>> { " today. " }
                    </p>
                </center>
            </div>
        </>
    }
}

#[function_component(SignUp)]
fn signup() -> Html {
    let history = use_history().unwrap();

    let onclick = Callback::once(move |r: Route| history.push(Route::SignUp));
    html! {
        <>
            <NavBar />

            <h1 style="margin-top:70px; text-align: center;">{ "Sign Up" }</h1>
            <div class="blue container">
                <p>{"something"}</p>
            </div>
        </>
    }
}

#[function_component(NotFound)]
fn not_found() -> Html {
    let history = use_history().unwrap();

    let onclick = Callback::once(move |r: Route| history.push(Route::SignUp));
    html! {
        <>
            <NavBar />
            <center><h1 style="margin-top:70px; text-align: center;">{ "404: Not Found" }</h1></center>
        </>
    }
}

fn switch(routes: &Route) -> Html {
    match routes {
        Route::Home => html! { <Home /> },
        Route::SignUp => html! { <SignUp /> },
        Route::Create => html! { <NotFound /> },
        Route::NotFound => html! { <NotFound /> },
    }
}

#[function_component(Main)]
fn app() -> Html {
    html! {
        <BrowserRouter>
            <Switch<Route> render={Switch::render(switch)} />
        </BrowserRouter>
    }
}

fn main() {
    yew::start_app::<Main>();
}

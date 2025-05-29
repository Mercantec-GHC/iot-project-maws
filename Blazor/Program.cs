using Blazor.Components;
using Microsoft.EntityFrameworkCore.Storage;
using Blazor.Services;
using DomainModels;
using API.Data;
using Microsoft.EntityFrameworkCore;

public class Program
{
    public static void Main(string[] args)
    {
        var builder = WebApplication.CreateBuilder(args);

        var connectionString = builder.Configuration.GetConnectionString("DefaultConnection")
            ?? Environment.GetEnvironmentVariable("DefaultConnection");

        // Configure DBContext
        builder.Services.AddDbContext<AppDBContext>(options => options.UseNpgsql(connectionString));

        // AuthenticationService Registration with relaxed cookie settings for development
        builder.Services.AddAuthentication("Cookies")
            .AddCookie("Cookies", options =>
            {
                if (builder.Environment.IsDevelopment())
                {
                    options.Cookie.SecurePolicy = CookieSecurePolicy.SameAsRequest; // Allow HTTP in development
                    options.Cookie.SameSite = SameSiteMode.Lax; // More permissive for cross-origin
                }
                else
                {
                    options.Cookie.SecurePolicy = CookieSecurePolicy.Always; // HTTPS only in production
                    options.Cookie.SameSite = SameSiteMode.Strict;
                }
            });

        // Register AppState
        builder.Services.AddScoped<AppState>();

        builder.Services.AddScoped<MailServices>();

        // Add services to the container.
        builder.Services.AddRazorComponents()
            .AddInteractiveServerComponents();

        // Configure HttpClient to use ApiBaseUrl from configuration
        builder.Services.AddHttpClient("API", client =>
        {
            var apiBaseUrl = builder.Configuration["ApiBaseUrl"] ?? 
                     (builder.Environment.IsDevelopment() 
                         ? "http://localhost:5021/" 
                         : "https://tama-town-central-api.onrender.com/");
            client.BaseAddress = new Uri(apiBaseUrl);
        });

        builder.Services.AddScoped(sp => sp.GetRequiredService<IHttpClientFactory>().CreateClient("API"));

        var app = builder.Build();

        // Configure the HTTP request pipeline.
        if (!app.Environment.IsDevelopment())
        {
            app.UseExceptionHandler("/Error");
            app.UseHsts();
        }

        // Conditional HTTPS redirection
        if (!app.Environment.IsDevelopment())
        {
            app.UseHttpsRedirection();
        }

        app.UseStaticFiles();
        app.UseAntiforgery();
        app.MapRazorComponents<App>()
            .AddInteractiveServerRenderMode();

        app.Run();
    }
}